#ifndef TIMER_H_
#define TIMER_H_

#include<functional>
#include<chrono>
#include<thread>
#include<atomic>
#include<memory>
#include<mutex>

#include<condition_variable>

namespace util {
	class Timer {

	public:

		Timer() :expired_(true) {
		}


		~Timer() {
			Expire();
		}
		
        void SetInterval(int interval) {
            interval_ = interval;
        }

		void StartTimer(int interval, std::function<void()> task) {
            interval_ = interval;
            expired_ = false;
			std::thread([this, task]() {
                {
                    std::unique_lock<std::mutex> locker(mutex_);
                    while (!expired_) {
                        expired_cond_.wait_for(locker, std::chrono::milliseconds(interval_));
                        if (expired_) {
                            break;
                        }

                        task();
                    }
                }
                std::unique_lock<std::mutex> locker_exit(mutex_exit_);
                exit_cond_.notify_one();
			}).detach();
		}
		

		void Expire() {

			std::unique_lock<std::mutex> locker_exit(mutex_exit_);
			{
				std::unique_lock<std::mutex> locker(mutex_);
				if (expired_) {
					return;
				}
				expired_ = true;
				expired_cond_.notify_one();
			}

			exit_cond_.wait(locker_exit);
		}

		
		template<typename callable, class... arguments>
		void SyncWait(int after, callable&& f, arguments&&... args) {
			std::function<typename std::result_of<callable(arguments...)>::type()> task
			(std::bind(std::forward<callable>(f), std::forward<arguments>(args)...));

			std::this_thread::sleep_for(std::chrono::milliseconds(after));
			task();
		}

		template<typename callable, class... arguments>
		void AsyncWait(int after, callable&& f, arguments&&... args) {
			std::function<typename std::result_of<callable(arguments...)>::type()> task
			(std::bind(std::forward<callable>(f), std::forward<arguments>(args)...));
					   
			std::thread([after, task]() {
				std::this_thread::sleep_for(std::chrono::milliseconds(after));
				task();
			}).detach();
		}
			   
	private:
		bool					expired_;
		std::mutex				mutex_;
		std::condition_variable expired_cond_;
		std::mutex				mutex_exit_;
		std::condition_variable exit_cond_;
        int                     interval_;
	};
}


#if 0
////////////test//////////////
int main() {
	Timer t;
	//周期性执行定时任务
	t.StartTimer(1000, std::bind(EchoFunc, "hello world!"));
	std::this_thread::sleep_for(std::chrono::seconds(4));
	std::cout << "try to expire timer!" << std::endl;
	t.Expire();
	

	//周期性执行定时任务
	t.StartTimer(1000, std::bind(EchoFunc, "hello c++11!"));
	std::this_thread::sleep_for(std::chrono::seconds(4));
	std::cout << "try to expire timer!" << std::endl;
	t.Expire();
	

	std::this_thread::sleep_for(std::chrono::seconds(2));
	   
	//只执行一次定时任务
	//同步
	t.SyncWait(1000, EchoFunc, "hello world!");
	//异步
	t.AsyncWait(1000, EchoFunc, "hello c++11!");
	
	std::this_thread::sleep_for(std::chrono::seconds(2));
	   
	return 0;
}
#endif

#endif
