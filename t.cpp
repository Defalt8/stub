// #include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <ctime>

class Time
{
	public:
		struct timespec
		{
			long long tv_sec;
			long tv_nsec;
		};
	public:
		Time();
	public:
		bool set();
		double deltaTime() const;
	private:
		timespec _checkpoint;
};

///////////////
// Time

Time::Time() :
	_checkpoint{0,0}
{}

bool Time::set()
{
	::timespec ts;
	if(TIME_UTC != timespec_get(&ts, TIME_UTC)) return false;
	_checkpoint.tv_sec = ts.tv_sec;
	_checkpoint.tv_nsec = ts.tv_nsec;
	return true;
}

double Time::deltaTime() const
{
	::timespec ts;
	if(TIME_UTC != timespec_get(&ts, TIME_UTC)) return -1.0;
	long nsec;
	if(ts.tv_nsec >= _checkpoint.tv_nsec) nsec = ts.tv_nsec - _checkpoint.tv_nsec;
	else nsec = ts.tv_nsec + (999999999 - _checkpoint.tv_nsec);
	return static_cast<double>(ts.tv_sec - _checkpoint.tv_sec) + 1e-9*static_cast<double>(nsec);
}

/////////
// main

int main(int argc, char* argv[])
{
	int N = 3;
	long iterations = 1000000L;
	if(argc > 1) N = atoi(argv[1]);
	if(argc > 2) iterations = atol(argv[2]);
	/********************************/
	Time t;
	double dt, tdt = 0.0, tmax = 0.0, tmin = 1e+28, instruction_count = 4.0;
	int n;
	for(n=0; n<N; ++n)
	{
		t.set();
		/*
		* i=0	; constant time so we ignore it
		* i<0	; 1 - comparision
		* i=i+1	; 2 - assignment and operation
		* jmpc	; 1 - hidden conditional jump instruction
		*/
		for(register long i=0; i<iterations; i=i+1); // instruction_count == 4.0;
		dt = t.deltaTime();
		if(dt < 0.0) continue;
		tdt += dt;
		if(dt < tmin) tmin = dt;
		if(dt > tmax) tmax = dt;
		printf("Test %d: %.3lf GHz\n", (n+1), (static_cast<double>(iterations) / dt * instruction_count / 1e+9));
	}
	printf("min: %.3lf GHz | max: %.3lf GHz | average: %.3lf GHz\n", 
		(static_cast<double>(iterations) / tmax * instruction_count / 1e+9), 
		(static_cast<double>(iterations) / tmin * instruction_count / 1e+9), 
		(static_cast<double>(iterations) / (tdt / n) * instruction_count / 1e+9)
	);
	return 0;
}
