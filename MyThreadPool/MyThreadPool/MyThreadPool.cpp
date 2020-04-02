#include <condition_variable>
#include <functional>
#include <vector>
#include <thread>
#include <queue>
#include <iostream>

class ThreadPoolClass {
public :
	using Task = std::function<void()>;	
	explicit ThreadPoolClass(std::size_t NumThreads) {
		start(NumThreads);
	}
	~ThreadPoolClass() {
		stop();
	}

	void enqueue(Task task) {
		{
			std::unique_lock<std::mutex> lock{ MEventMutex };
			MTasks.emplace(std::move(task));
		}
		MEventVar.notify_one();
	}

private:
	std::vector<std::thread> MThreads;
	std::condition_variable MEventVar;
	std::mutex MEventMutex;
	std::queue<Task> MTasks;

	bool stopping = false;
	void start(std::size_t NumThreads) {
		for (size_t i = 0u; i < NumThreads; ++i)
		{
			MThreads.emplace_back([=] {
				while (true) {
					Task task;
					{
						std::unique_lock<std::mutex> lock{ MEventMutex };
						MEventVar.wait(lock, [=] {return stopping || !MTasks.empty(); });

						if (stopping && MTasks.empty())
							break;

						task = std::move(MTasks.front());
						MTasks.pop();
					}
					task();
				}

			});
		}
	}

	void stop() noexcept {
		{
			std::unique_lock<std::mutex> lock{ MEventMutex };
			stopping = true;
		}
		MEventVar.notify_all();

		for (auto &thread : MThreads)
			thread.join();	
	}
};

int main()
{
	{
		ThreadPoolClass pool{ 36 };

		pool.enqueue([] {
			std::cout << "1" << std::endl;
		});
		pool.enqueue([] {
			std::cout << "2" << std::endl;
		});
	}
	return 0;
}