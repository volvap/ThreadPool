#include <condition_variable>
#include <functional>
#include <vector>
#include <thread>

class ThreadPoolClass {
public :
	explicit ThreadPoolClass(std::size_t NumThreads) {
		start(NumThreads);
	}
	~ThreadPoolClass() {
		stop();
	}

private:
	std::vector<std::thread> MThreads;

	std::condition_variable MEventVar;

	std::mutex MEventMutex;
	bool stopping = false;
	void start(std::size_t NumThreads) {
		for (size_t i = 0u; i < NumThreads; ++i)
		{
			MThreads.emplace_back([=] {
				while (true) {
					std::unique_lock<std::mutex> lock{ MEventMutex };
					MEventVar.wait(lock, [=] {return stopping; });

					if (stopping)
						break;
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
	ThreadPoolClass(36);
	return 0;
}