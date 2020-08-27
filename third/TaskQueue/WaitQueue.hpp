//
//  WaitQueue.h
//  TaskLoop
//
//  Created by pansafeimager on 15/12/9.
//  Copyright © 2015年 imager. All rights reserved.
//

#ifndef WaitQueue_h
#define WaitQueue_h
#include <queue>

#include <condition_variable>

namespace task {
    
    template<typename T>
    class RunnerQueue: public std::queue<T>
    {
    public:
        void Swap(RunnerQueue<T>& queue)
        {
            this->c.swap(queue.c);
        }
    };

    template<typename T>
    class WaitQueue
    {
        public:
        
        typedef RunnerQueue<T> QueueType;
        typedef typename QueueType::value_type value_type;

        WaitQueue():_waitqueue(new QueueType)
        {
            
        }
        
        ~WaitQueue(){}

        void Add(value_type v)
        {
            {
                std::lock_guard<std::mutex> lock(_mutex_queue);
                _waitqueue->push(v);
            }
            Notify();
        }
        
        //is empty
        bool Empty()
        {
            std::lock_guard<std::mutex> lock(_mutex_queue);
            return _waitqueue->empty();
        }
        //get size of waitqueue
        size_t Size()
        {
            std::lock_guard<std::mutex> lk(_mutex_queue);
            return _waitqueue->size();
        }
        
        //swap queue
        size_t ReloadWaitQueue(QueueType& queue)
        {
            std::lock_guard<std::mutex> lk(_mutex_queue);
            _waitqueue->Swap(queue);
            return queue.size();
        }
        
        //wait for new task
        void WaitforWork()
        {
            //std::unique_lock<std::mutex> lk(_mutex_con);
			std::unique_lock<std::mutex> lk(_mutex_queue);
			//while (_waitqueue->empty())
            if (_waitqueue->empty())
            {
                _con_var.wait_for(lk,std::chrono::milliseconds(40));
            }
            //lk.release();
        }
        //notify all waiting thread
        void Notify()
        {
            _con_var.notify_all();
        }
    protected:
        QueueType* _waitqueue;
        std::mutex _mutex_queue;
        std::mutex _mutex_con;
        std::condition_variable _con_var;
    };
}

#endif /* WaitQueue_h */
