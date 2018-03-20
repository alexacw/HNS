/*
 * test.h
 *
 *  Created on: 19 Mar 2018
 *      Author: acw
 */

#ifndef TEST_HPP_
#define TEST_HPP_
#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"

#include "shell.h"
#include "chprintf.h"

#include "usbcfg.h"

int testFun();

class TestClass {
public:
	TestClass(int x) :
			x(x) {
	}
	;
	int x;
};

#endif /* TEST_HPP_ */
