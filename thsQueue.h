#pragma once

#include <atomic>
#include "spinlock_mutex.h"

#include <iostream>
#include <mutex>

//#define COMPARE_TO_STD

#if defined(COMPARE_TO_STD)
#include <mutex>
#include <queue>
#endif

/*
	Many producers to One consumer queue
*/
template <class T, size_t N, size_t ThreadNum>
class m2oQueue final
{
public:
	m2oQueue() {}
	~m2oQueue() {}

#if defined(COMPARE_TO_STD)
	std::mutex mtx;
	std::queue<T> q;
#endif

	bool push(const T& v)
	{
#if defined(COMPARE_TO_STD)
		std::lock_guard<std::mutex> l(mtx);// rm it
		if (q.size() > N)
			return false;
		q.push(v);
		return true;
#endif

		if (m_writeHead.load() - m_readHead.load() > N)
			return false; // no place

		// it's possible that queue is almost full and ThreadNum of threads entered,
		// it still has enough place to store all values.

		uint64_t ind = m_writeHead.fetch_add(1);

		m_arr[ind % sizeofArr()] = v;

		while (m_writeTail.load() != ind);
		++m_writeTail;

		return true;
	}
	bool pop(T& out_v)
	{
#if defined(COMPARE_TO_STD)
		std::lock_guard<std::mutex> l(mtx);// rm it
		if (q.size() == 0)
			return false;
		out_v = q.front();
		q.pop();
		return true;
#endif

		if(m_readHead.load() == m_writeTail.load())
			return false; // empty

		out_v = m_arr[m_readHead.load() % sizeofArr()];
		++m_readHead;

		return true;
	}

private:
	constexpr size_t sizeofArr()const { return sizeof(m_arr) / sizeof(m_arr[0]); }


	T m_arr[N + ThreadNum + 1];
	std::atomic<uint64_t> m_writeHead{ 0 };
	std::atomic<uint64_t> m_writeTail{ 0 };
	std::atomic<uint64_t> m_readHead{ 0 };
};


/*
	Many producers to Many consumers queue
*/
template <class T, size_t N, size_t ThreadNum>
class m2mQueue final
{
public:
	m2mQueue() = default;
	~m2mQueue() = default;

#if defined(COMPARE_TO_STD)
	std::mutex mtx;
	std::queue<T> q;
#endif

	bool push(const T& v)
	{
#if defined(COMPARE_TO_STD)
		std::lock_guard<std::mutex> l(mtx);// rm it
		if (q.size() > N)
			return false;
		q.push(v);
		return true;
#endif

		if (m_readHead.load() > m_writeTail.load())
			*(static_cast<int*>(0)) = 0;

		if (m_writeHead.load() - m_readTail.load() > N)
			return false; // no place

		// it's possible that queue is almost full and ThreadNum of threads entered,
		// it still has enough place to store all values.

		uint64_t ind = m_writeHead.fetch_add(1);

		m_arr[ind % sizeofArr()] = v;

		while (m_writeTail.load() != ind);
		++m_writeTail;

		//printState();

		return true;
	}
	bool pop(T& out_v)
	{
#if defined(COMPARE_TO_STD)
		std::lock_guard<std::mutex> l(mtx);// rm it
		if (q.size() == 0)
			return false;
		out_v = q.front();
		q.pop();
		return true;
#endif

		uint64_t ind{ 0 };
		{
			// tried with RAII std::lock_guard and my own lock
			// performance drops drastically 
			m_mtx.lock();
			if (m_readHead.load() < m_writeTail.load())
			{
				ind = m_readHead.fetch_add(1);
				m_mtx.unlock();
			}
			else
			{
				m_mtx.unlock();
				return false; // empty
			}

			if (m_readHead.load() > m_writeTail.load())
				*(static_cast<int*>(0)) = 0;
		}

		out_v = m_arr[ind % sizeofArr()];

		while (m_readTail.load() != ind);
		++m_readTail;

		//printState();

		return true;
	}

private:
	constexpr size_t sizeofArr()const { return sizeof(m_arr) / sizeof(m_arr[0]); }

	void printState()
	{
		static std::mutex mtx;
		std::lock_guard<std::mutex> l(mtx);

		std::cout 
			<< "rT: " << m_readTail.load() 
			<< ", rH: " << m_readHead.load()
			<< ", wT: " << m_writeTail.load()
			<< ", wH: " << m_writeHead.load()
			<< std::endl;
	}

	T m_arr[N + ThreadNum + 1];
	std::atomic<uint64_t> m_writeHead{ 0 };
	std::atomic<uint64_t> m_writeTail{ 0 };
	std::atomic<uint64_t> m_readHead{ 0 };
	std::atomic<uint64_t> m_readTail{ 0 };

	spinlock_mutex m_mtx;
};

