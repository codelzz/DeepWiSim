// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FUdpServer.h"
#include "UdpServerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DEEPWISIM_API UUdpServerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUdpServerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void Send(const FString& Data);
	void StartServer();
	void ShutdownServer();
	bool IsServerValid();

public:
	UFUNCTION(BlueprintCallable, Category = "Udp Server")
	FString GetServerEndpoint()
	{
		return FString::Printf(TEXT("%s:%d"), *ServerIP, ServerPort);
	}

	UFUNCTION(BlueprintCallable, Category = "Udp Server")
	FString GetClientEndpoint()
	{
		return FString::Printf(TEXT("%s:%d"), *ClientIP, ClientPort);
	}

public:
	UPROPERTY(EditAnywhere, category = "Udp Server")
		FString ServerIP = TEXT("127.0.0.1");
	UPROPERTY(EditAnywhere, category = "Udp Server")
		uint16 ServerPort = 8888;
	UPROPERTY(EditAnywhere, category = "Udp Client")
		FString ClientIP = TEXT("127.0.0.1");
	UPROPERTY(EditAnywhere, category = "Udp Client")
		uint16 ClientPort = 8080;

protected:
	TSharedPtr<FUdpServer> Server;
};
