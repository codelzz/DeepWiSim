// Fill out your copyright notice in the Description page of Project Settings.


#include "UdpServerComponent.h"
#include "FUdpServer.h"

// Sets default values for this component's properties
UUdpServerComponent::UUdpServerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// ...
}


// Called when the game starts
void UUdpServerComponent::BeginPlay()
{
	Super::BeginPlay();
		StartServer();
}

void UUdpServerComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	switch (EndPlayReason)
	{
	case EEndPlayReason::Destroyed:
	{
		ShutdownServer();
		break;
	}
	}
}

// Called every frame
void UUdpServerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UUdpServerComponent::StartServer()
{
	Server = MakeShared<FUdpServer>(FUdpServer::ToEndpoint(ServerIP, ServerPort),FUdpServer::ToEndpoint(ClientIP, ClientPort));
	if (IsServerValid())
	{
		Server->StartListening();
	}
}

void UUdpServerComponent::ShutdownServer()
{
	if (IsServerValid())
	{
		Server->StopListening();
		Server.Reset();
		Server = nullptr;
	}
}

bool UUdpServerComponent::IsServerValid()
{
	return Server != nullptr && Server.IsValid();
}

void UUdpServerComponent::Send(const FString& Data)
{
	if (IsServerValid())
	{
		Server->Send(FUdpServer::ByteArrayFromString(Data).ToSharedRef());
	}
}


