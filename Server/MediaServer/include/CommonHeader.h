/*
 * CommonHeader.h
 *
 *  Created on: 2016年7月22日
 *      Author: max
 */

#ifndef INCLUDE_COMMONHEADER_H_
#define INCLUDE_COMMONHEADER_H_

#define FLAG_2_STRING(bFlag) bFlag?"OK":"Fail"
#define BOOL_2_STRING(bFlag) bFlag?"True":"False"
#define PULL_OR_PUSH_2_STRING(bFlag) bFlag?"Pull":"Push"

#include <memory>
#include <type_traits>
using namespace std;
namespace mediaserver {
// 支持普通指针
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// 支持动态数组
template<class T> inline
typename std::enable_if<is_array<T>::value && std::extent<T>::value == 0, std::unique_ptr<T>>::type
make_unique(size_t size){
    typedef typename std::remove_extent<T>::type U;
    return std::unique_ptr<T>(new U[size]());
}
}

#endif /* INCLUDE_COMMONHEADER_H_ */
