#pragma once
#include "SeekerEngine.h"

class SEFramework
{
public:
	virtual void Init();
	virtual void Shutdown();
	virtual void Update();
	virtual void Draw() = 0;
	virtual bool IsEndRequst() { return endRequst_; }
	virtual ~SEFramework() = default;

	void Run();

private:
	bool endRequst_ = false;

protected:
	SeekerEngine engine_;
};

