//
//  RunLoop.hpp
//  TaskLoop
//
//  Created by pansafeimager on 15/12/9.
//  Copyright © 2015 imager. All rights reserved.
//

#ifndef RunLoop_hpp
#define RunLoop_hpp

#include <thread>
#include <future>  

#include "Runner.hpp"
#include "WaitQueue.hpp"


namespace task
{
	
	class Runloop
    {
    public:
      
		void AddRunner(const Clouser& clouser)
		{	
			Clouser c(clouser);
			if (std::this_thread::get_id() == _taskThreadId) {
				c.Run();
				return;
			}

			_waitqueue.Add(clouser);
		}
				
		void AddSynRunner(const Clouser& clouser)
		{	
			Clouser c(clouser);
			if (std::this_thread::get_id() == _taskThreadId) {
				c.Run();
				return;
			}

			std::promise<bool> prom;
			std::future<bool> fu = prom.get_future();

			_waitqueue.Add(task::Clouser([&]() {
				c.Run();
				prom.set_value(true);
			}));

			fu.get();
		}

		void DoLoop()
		{
			do {
				if (_waitqueue.Empty() && !_isEnd) {
					_waitqueue.WaitforWork();
				}
				WaitQueue<Clouser>::QueueType queue;
				_waitqueue.ReloadWaitQueue(queue);
				
				while(!queue.empty()){
					Clouser clouser = queue.front();
					Schedule(clouser);
					queue.pop();
				}
			} while (!_isEnd);

			_promiseExit.set_value(true);
		}

		static Runloop* Create() {
			auto ploop = new Runloop();
			std::thread t(std::bind(&Runloop::DoLoop, ploop));
			ploop->SetTaskThreadId(t.get_id());
			t.detach();
			return ploop;
		}
		
		void Schedule(Clouser& clouser)
		{
			clouser.Run();
		}

		void Stop() {
			_isEnd = true;
			_waitqueue.Notify();
			std::future<bool> futureObj = _promiseExit.get_future();
			futureObj.get();
			delete this;
		}		

    protected:
        
        Runloop() {}
        ~Runloop() {}
				
	private:
		void SetTaskThreadId(const std::thread::id&	id) { _taskThreadId = id; }

	private:
        WaitQueue<Clouser>	_waitqueue;
		bool				_isEnd = false;
		std::thread::id		_taskThreadId;
		std::promise<bool>  _promiseExit;
    };    
    
}

#endif /* RunLoop_hpp */
