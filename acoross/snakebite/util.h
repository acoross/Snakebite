#ifndef SNAKEBITE_UTIL_H_
#define SNAKEBITE_UTIL_H_

#include <atomic>
#include <acoross/snakebite/win/WinWrapper.h>

namespace acoross {
namespace snakebite {

template <size_t Pcnt>
class MeanProcessTimeCheckerBase final
{
public:
	MeanProcessTimeCheckerBase(MeanProcessTimeCheckerBase&) = delete;
	MeanProcessTimeCheckerBase& operator=(MeanProcessTimeCheckerBase&) = delete;

	MeanProcessTimeCheckerBase(std::atomic<double>& mean)
		: start_tick_((double)::GetTickCount64())
		, mean_process_time_ms_(mean)
	{
		static_assert(Pcnt < 100, "MeanProcessTimeCheckerBase: Pcnt should < 100");
	}
	~MeanProcessTimeCheckerBase()
	{
		double diff_time_ms = (double)::GetTickCount64() - start_tick_;
		double new_mean_time = mean_process_time_ms_.load() * (double)Pcnt/100.0 + diff_time_ms * (100.0 - Pcnt)/100.0;
		mean_process_time_ms_.store(new_mean_time);
	}

	const double start_tick_{ 0 };
	std::atomic<double>& mean_process_time_ms_;
};

using MeanProcessTimeChecker = MeanProcessTimeCheckerBase<90>;

}
}
#endif //SNAKEBITE_UTIL_H_