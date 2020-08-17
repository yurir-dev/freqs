#include "tests.h"
#include "thsQueue.h"

#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>
#include <queue>

template <size_t N = 1>
struct Node
{
	Node(size_t n = 0)
	{
		for (size_t i = 0; i < sizeof(nums) / sizeof(nums[0]); ++i)
			nums[i] = n;
	}

	size_t val() const { return nums[0]; }
	bool verify()const
	{
		size_t val = nums[0];
		for (auto x : nums)
			if (val != x)
				return false;
		return true;
	}

	size_t nums[N];
};

using testNode = Node<1>;

struct stats
{
	std::atomic<size_t> totalPush{ 0 };
	std::atomic<size_t> pushSum{ 0 };
	std::atomic<size_t> totalPull{ 0 };
	std::atomic<size_t> pullSum{ 0 };

	void reset()
	{
		totalPush.store(0);
		pushSum.store(0);
		totalPull.store(0);
		pullSum.store(0);
	}
};

bool testM2OQueue()
{
	bool res{ true };
	const size_t threadSize = std::thread::hardware_concurrency();
	m2oQueue<testNode, 1024, 32> q;


	stats stats;
	std::mutex mtx;
	std::atomic<bool> endPush{ false };
	std::atomic<bool> endPop{ false };
	std::atomic<size_t> val2push{ 0 };

	auto pusher = [&]()
	{
		size_t good_push{ 0 }, bad_push{ 0 };
		size_t val = 0;
		size_t lastPushed{ 0 };
		bool pushed{ true };

		while (!endPush.load())
		{
			if (pushed)
				val = val2push.fetch_add(1);

			pushed = q.push(testNode(val));
			if (pushed)
			{
				lastPushed = val;
				stats.pushSum += lastPushed;
				good_push++;
			}
			else
				bad_push++;
		}

		stats.totalPush += good_push;

		std::lock_guard<std::mutex> l(mtx);
		std::cout << "pusher: good_pushs: " << good_push << ", bad_pushs: " << bad_push << ", last pushed: " << lastPushed << std::endl;
	};

	auto poper = [&]()
	{
		size_t lastPop{ 0 };

		size_t good_pop{ 0 }, bad_pop{ 0 };
		while (!endPop.load())
		{
			testNode n;
			if (q.pop(n))
			{
				good_pop++;
				lastPop = n.val();
				stats.pullSum += lastPop;
			}
			else
				bad_pop++;
		}

		stats.totalPull += good_pop;

		std::lock_guard<std::mutex> l(mtx);
		std::cout << "poper: good_pops: " << good_pop << ", bad_pops: " << bad_pop << ", last pop: " << lastPop << std::endl;
	};

	{
		std::cout << __FUNCTION__ << " Test : Many pushes then pulls" << std::endl;
		std::cout << "-------------------------------------------------" << std::endl;

		stats.reset();
		val2push.store(0);
		endPop.store(false);
		endPush.store(false);

		std::vector<std::thread> threads; threads.resize(threadSize);
		for (size_t i = 0; i < threads.size() - 1; i++)
		{
			threads[i] = std::thread(pusher);
		}
		threads[threads.size() - 1] = std::thread(poper);

		std::this_thread::sleep_for(std::chrono::milliseconds(10000));
		endPush.store(true);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		endPop.store(true);

		for (auto& t : threads)
			t.join();

		std::cout << std::endl;
		std::cout << "totalPops: " << stats.totalPull.load() << ", total push: " << stats.totalPush.load() << " , verdict : " << (stats.totalPush.load() == stats.totalPull.load() ? "OK" : "FAIL") << std::endl;
		std::cout << "pushed sum: " << stats.pushSum.load() << ", pulled sum: " << stats.pullSum.load() << " , verdict : " << (stats.pushSum.load() == stats.pullSum.load() ? "OK" : "FAIL") << std::endl;

		res = (stats.totalPush.load() == stats.totalPull.load()) && (stats.pushSum.load() == stats.pullSum.load());

		std::cout << " ---- End ----" << std::endl << std::endl << std::endl;
	}

	if (!res)
		return res;

	{
		std::cout << __FUNCTION__ << " Test : pulls then many pushes" << std::endl;
		std::cout << "-------------------------------------------------" << std::endl;

		stats.reset();
		val2push.store(0);
		endPop.store(false);
		endPush.store(false);

		std::vector<std::thread> threads; threads.resize(threadSize);
		threads[0] = std::thread(poper);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		for (size_t i = 1; i < threads.size(); i++)
		{
			threads[i] = std::thread(pusher);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10000));
		endPush.store(true);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		endPop.store(true);

		for (auto& t : threads)
			t.join();

		std::cout << std::endl;
		std::cout << "totalPops: " << stats.totalPull.load() << ", total push: " << stats.totalPush.load() << " , verdict : " << (stats.totalPush.load() == stats.totalPull.load() ? "OK" : "FAIL") << std::endl;
		std::cout << "pushed sum: " << stats.pushSum.load() << ", pulled sum: " << stats.pullSum.load() << " , verdict : " << (stats.pushSum.load() == stats.pullSum.load() ? "OK" : "FAIL") << std::endl;

		res = (stats.totalPush.load() == stats.totalPull.load()) && (stats.pushSum.load() == stats.pullSum.load());

		std::cout << " ---- End ----" << std::endl << std::endl << std::endl;
	}

	return res;
}


bool testM2MQueue()
{
	bool res{ true };
	const size_t threadSize = std::thread::hardware_concurrency();
	m2mQueue<testNode, 1024, 32> q;


	stats stats;
	std::mutex mtx;
	std::atomic<bool> endPush{ false };
	std::atomic<bool> endPop{ false };
	std::atomic<size_t> val2push{ 0 };

	auto pusher = [&]()
	{
		size_t good_push{ 0 }, bad_push{ 0 };
		size_t val = 0;
		size_t lastPushed{ 0 };
		bool pushed{ true };

		while (!endPush.load())
		{
			if (pushed)
				val = val2push.fetch_add(1);

			pushed = q.push(testNode(val));
			if (pushed)
			{
				lastPushed = val;
				stats.pushSum += lastPushed;
				good_push++;
			}
			else
				bad_push++;
		}

		stats.totalPush += good_push;

		std::lock_guard<std::mutex> l(mtx);
		std::cout << "pusher: good_pushs: " << good_push << ", bad_pushs: " << bad_push << ", last pushed: " << lastPushed << std::endl;
	};

	auto poper = [&]()
	{
		size_t lastPop{ 0 };

		size_t good_pop{ 0 }, bad_pop{ 0 };
		while (!endPop.load())
		{
			testNode n;
			if (q.pop(n))
			{
				good_pop++;
				lastPop = n.val();
				stats.pullSum += lastPop;
			}
			else
				bad_pop++;
		}

		stats.totalPull += good_pop;

		std::lock_guard<std::mutex> l(mtx);
		std::cout << "poper: good_pops: " << good_pop << ", bad_pops: " << bad_pop << ", last pop: " << lastPop << std::endl;
	};

	{
		std::cout << __FUNCTION__ << " Test : Many pushes then pulls" << std::endl;
		std::cout << "-------------------------------------------------" << std::endl;

		stats.reset();
		val2push.store(0);
		endPop.store(false);
		endPush.store(false);

		std::vector<std::thread> threads; threads.resize(threadSize);
		for (size_t i = 0; i < threads.size() / 2; i++)
			threads[i] = std::thread(pusher);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		for (size_t i = threads.size() / 2; i < threads.size(); i++)
			threads[i] = std::thread(poper);

		std::this_thread::sleep_for(std::chrono::milliseconds(10000));
		endPush.store(true);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		endPop.store(true);

		for (auto& t : threads)
		{
			if(t.joinable())
				t.join();
		}
		std::cout << std::endl;
		std::cout << "totalPops: " << stats.totalPull.load() << ", total push: " << stats.totalPush.load() << " , verdict : " << (stats.totalPush.load() == stats.totalPull.load() ? "OK" : "FAIL") << std::endl;
		std::cout << "pushed sum: " << stats.pushSum.load() << ", pulled sum: " << stats.pullSum.load() << " , verdict : " << (stats.pushSum.load() == stats.pullSum.load() ? "OK" : "FAIL") << std::endl;

		res = (stats.totalPush.load() == stats.totalPull.load()) && (stats.pushSum.load() == stats.pullSum.load());

		std::cout << " ---- End ----" << std::endl << std::endl << std::endl;
	}

	if (!res)
		return res;

	{
		std::cout << __FUNCTION__ << " Test : many pulls then many pushes" << std::endl;
		std::cout << "-------------------------------------------------" << std::endl;

		stats.reset();
		val2push.store(0);
		endPop.store(false);
		endPush.store(false);

		std::vector<std::thread> threads; threads.resize(threadSize);
		for (size_t i = 0; i < threads.size() / 2; i++)
			threads[i] = std::thread(poper);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		for (size_t i = threads.size() / 2; i < threads.size(); i++)
			threads[i] = std::thread(pusher);

		std::this_thread::sleep_for(std::chrono::milliseconds(10000));
		endPush.store(true);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		endPop.store(true);

		for (auto& t : threads)
			t.join();

		std::cout << std::endl;
		std::cout << "totalPops: " << stats.totalPull.load() << ", total push: " << stats.totalPush.load() << " , verdict : " << (stats.totalPush.load() == stats.totalPull.load() ? "OK" : "FAIL") << std::endl;
		std::cout << "pushed sum: " << stats.pushSum.load() << ", pulled sum: " << stats.pullSum.load() << " , verdict : " << (stats.pushSum.load() == stats.pullSum.load() ? "OK" : "FAIL") << std::endl;

		res = (stats.totalPush.load() == stats.totalPull.load()) && (stats.pushSum.load() == stats.pullSum.load());

		std::cout << " ---- End ----" << std::endl << std::endl << std::endl;
	}

	return res;
}