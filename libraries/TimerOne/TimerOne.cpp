/*
  TimerOne.cpp - DM&P Vortex86 TimerOne library
  Copyright (c) 2014 Android Lin <acen@dmp.com.tw>. All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  (If you need a commercial license, please contact soc@dmp.com.tw 
   to get more information.)
*/
#ifndef TIMERONE_cpp
#define TIMERONE_cpp

#include "TimerOne.h"

TimerOne Timer1;

void TimerOne::initialize(long microseconds)
{
	this->setPeriod(microseconds);
}

void TimerOne::setPeriod(long microseconds)
{
	this->periodMicroseconds = microseconds > 0 ? microseconds : 1;
}

void TimerOne::setPwmDuty(char pin, int duty)
{
	return;
}

void TimerOne::pwm(char pin, int duty, long microseconds)
{
	return;
}

void TimerOne::disablePwm(char pin)
{
	return;
}

void TimerOne::attachInterrupt(void (*isr)(), long microseconds)
{
	if(microseconds < 0)
		microseconds = this->periodMicroseconds;

	attachTimerInterrupt(128 ,isr, microseconds);
}

void TimerOne::detachInterrupt()
{
	detachTimerInterrupt();
}

void TimerOne::resume()
{
	return;
}

void TimerOne::restart()
{
	return;
}

void TimerOne::start()
{
	return;
}

void TimerOne::stop()
{
	return;
}

unsigned long TimerOne::read()
{
	return 0;
}

#endif
