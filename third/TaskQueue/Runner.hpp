//
//  Runner.hpp
//  TaskLoop
//
//  Created by pansafeimager on 15/12/9.
//  Copyright © 2015 imager. All rights reserved.
//

#ifndef Runner_hpp
#define Runner_hpp

#include <functional>

namespace task {
    
    template<class T>
    class Runner;
    
    template<class R,class... Args>
    class Runner<R(Args...)>
    {
        typedef R return_type;
        typedef std::function<R(Args...)> func_type;
        
        public:
        
        Runner(const func_type& func)
        :_func(func)
        {
            
        }
        
        Runner(const Runner& other)
        {
            _func = other._func;
        }
    
        Runner& operator=(const Runner& other)
        {
            _func = other._func;
        }
        return_type Run()
        {
            return _func();
        }
    protected:
        func_type _func;
    };
    typedef Runner<void(void)> Clouser;
}

#endif /* Runner_hpp */
