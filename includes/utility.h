/*****************************************************************************
 * This file is part of the Linux Camera Tool 
 * Copyright (c) 2020 Leopard Imaging Inc.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *																		    														 
 *  This is the a common used utility library 								 
 *																			 
 *  Author: Danyu L															 
 *  Last edit: 2019/08														 
 *****************************************************************************/
#pragma once
#include <chrono> //high resolution clock
#include <iostream>

/**
 * timer used for benchmark ISP performance
 */
class Timer
{
public:
	Timer()
	{
		m_start_time_point = std::chrono::high_resolution_clock::now();
	}
	~Timer()
	{
		Stop();
	}
	void Stop()
	{
		auto end_time_point = std::chrono::high_resolution_clock::now();
		auto start = std::chrono::time_point_cast<std::chrono::microseconds>
			(m_start_time_point).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::microseconds>
			(end_time_point).time_since_epoch().count();
		auto duration = end - start;
		double ms = duration *0.001;
		double second = ms * 0.001;
		//std::cout << "Elapsed time："<< duration << "us (" << ms << "ms)\n";
		std::cout << "Elapsed time："<< second << "s\n";
	}

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_start_time_point;
};

