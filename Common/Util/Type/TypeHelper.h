//
// Created by yjz on 2022/1/7.
//

#ifndef GAMEKEEPER_TYPEHELPER_H
#define GAMEKEEPER_TYPEHELPER_H
namespace Helper
{
    namespace Type
    {
        template<typename T>
        struct IsValueType { constexpr static bool Value = false; };
        template<> struct  IsValueType<int> { constexpr static bool Value = true;};
        template<> struct  IsValueType<char> { constexpr static bool Value = true;};
        template<> struct  IsValueType<bool> { constexpr static bool Value = true;};
        template<> struct  IsValueType<short> { constexpr static bool Value = true;};
        template<> struct  IsValueType<float> { constexpr static bool Value = true;};
        template<> struct  IsValueType<long> { constexpr static bool Value = true;};
        template<> struct  IsValueType<long long> { constexpr static bool Value = true;};
        template<> struct  IsValueType<double> { constexpr static bool Value = true;};
        template<> struct  IsValueType<unsigned int> { constexpr static bool Value = true;};
        template<> struct  IsValueType<unsigned short> { constexpr static bool Value = true;};
        template<> struct  IsValueType<unsigned long> { constexpr static bool Value = true;};
        template<> struct  IsValueType<unsigned long long> { constexpr static bool Value = true;};
    }
}
#endif //GAMEKEEPER_TYPEHELPER_H
