//
//  resource_c_wrapper.hpp
//  shmht
//
//  Created by Wang Zhang on 3/23/20.
//  Copyright © 2020 Wang Zhang. All rights reserved.
//

#ifndef resource_c_wrapper_hpp
#define resource_c_wrapper_hpp

#ifdef __cplusplus                     
extern "C" {
#endif

    void RNM_init(const int, const int);
    void print_rnodes(void);
    void print_gmem(void);
    void add_gmem(const int, const long, unsigned long long, const unsigned int);

#ifdef __cplusplus
}
#endif

#endif /* resource_c_wrapper_hpp */