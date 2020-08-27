//
//  main.cpp
//  TaskLoop
//
//  Created by pansafeimager on 15/12/9.
//  Copyright © 2015年 imager. All rights reserved.
//

#include <iostream>
#include "RunLoop.h"

void Func()
{
    std::cout<<"hello world!" << std::endl;
}

int Add(int a, int b){
    std::cout<< "call Add()" << std::endl;
    return a + b;
}
int main(int argc, const char * argv[]) {
    
    task::Runloop* loop = task::Runloop::Create();

    task::Clouser clouser([]{Add(1,2);});
    
    loop->AddRunner(clouser);
    loop->AddRunner(clouser);
    
    while(1);
    return 1;
}
