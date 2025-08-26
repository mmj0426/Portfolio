// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlowControl.generated.h"

DECLARE_DELEGATE(FSimpleDelegate);

USTRUCT()
struct FFlowControl
{
	GENERATED_BODY()

protected:
	bool bOpen;

	FSimpleDelegate OnEnter;

public:
	FFlowControl() : bOpen(false) { }
	FFlowControl(FSimpleDelegate EnterDelegate, bool StartClosed = false) : bOpen(!StartClosed), OnEnter(EnterDelegate) { }

	virtual ~FFlowControl() { };

	virtual void Enter() {
		if (bOpen)
		{
			OnEnter.ExecuteIfBound();
		}
	}
	virtual bool IsOpen() const { return bOpen; }
};

USTRUCT()
struct FFCDoOnce : public FFlowControl
{
	GENERATED_BODY()

public:
	FFCDoOnce() { }
	FFCDoOnce(FSimpleDelegate EnterDelegate, bool StartClosed = true) : FFlowControl(EnterDelegate, StartClosed) { }

	virtual ~FFCDoOnce() { };

	void Enter() final { Super::Enter(); bOpen = false; }

	void Reset() { bOpen = true; }
};

USTRUCT()
struct FFCGate : public FFlowControl
{
	GENERATED_BODY()

public:
	FFCGate() { }
	FFCGate(FSimpleDelegate EnterDelegate, bool StartClosed = true) : FFlowControl(EnterDelegate, StartClosed) { }

	virtual ~FFCGate() { };

	void Open() { bOpen = true; };
	void Close() { bOpen = false; };
};
