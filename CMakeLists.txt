cmake_minimum_required(VERSION 4.0)
project(OnlineShopping)

set(CMAKE_CXX_STANDARD 17)

add_executable(OnlineShopping main.cpp
        Product.cpp
        Product.h
        ProductManager.cpp
        ProductManager.h
        ShoppingCart.cpp
        ShoppingCart.h)
