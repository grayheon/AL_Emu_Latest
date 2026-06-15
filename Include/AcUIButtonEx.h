#pragma once

#include "AcUIButton.h"

class AcUIButtonEx : public AcUIButton
{
public:
	AcUIButtonEx();
	virtual ~AcUIButtonEx();

	virtual	BOOL HitTest( INT32 x , INT32 y	);

};
