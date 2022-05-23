// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Networking.h"

/**
 * An Udp Server 
 */
class DEEPWISIM_API FUdpServer
	: public TSharedFromThis<FUdpServer>
{
public:
	FUdpServer(const FIPv4Endpoint& InServerEndpoint, const FIPv4Endpoint& InClientEndpoint)
		: ClientEndpoint(InClientEndpoint)
		, ServerEndpoint(InServerEndpoint)
		, SocketSubSystem(ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM))
	{
	}
	~FUdpServer()
	{
		if (Sender != nullptr)
		{
			if (Sender.IsValid()) {	Sender.Reset();}
			Sender = nullptr;
		}
		if (Receiver != nullptr)
		{
			if (Receiver.IsValid()) { Receiver.Reset();}
			Receiver = nullptr;
		}
		if (Socket != nullptr)
		{
			SocketSubSystem->DestroySocket(Socket);
			Socket = nullptr;
		}
	}

public:
	/** Get Class Name */
	FString GetName() const
	{
		return TEXT("UdpServer");
	}

public:

	/**
	* Convert IP and Port to FIPv4Endpoint format
	* 
	* @param IP
	* @param Port
	* @return Endpoint
	*/
	static FIPv4Endpoint ToEndpoint(const FString& InIP, uint16 InPort)
	{
		FIPv4Address IPAddress;
		FIPv4Address::Parse(InIP, IPAddress);
		FIPv4Endpoint Endpoint(IPAddress, InPort);
		return Endpoint;
	}

	static FString StringFromTArrayReader(const FArrayReaderPtr& Reader)
	{
		return FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(Reader->GetData())));
	}

	static FString StringFromBytesSharedPtr(const TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe>& BytesPtr)
	{
		if (BytesPtr.IsValid())
		{
			return FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(BytesPtr->GetData())));
		}
		return TEXT("");
	}

	static FString StringFromBytes(const TArray<uint8>& Bytes)
	{
		return FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(Bytes.GetData())));
	}

	static TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ByteArrayFromString(const FString& String)
	{
		TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> Bytes = MakeShared<TArray<uint8>, ESPMode::ThreadSafe>();
		if (Bytes.IsValid())
		{
			Bytes->SetNum(String.Len());
			memcpy(Bytes->GetData(), TCHAR_TO_ANSI(*String), String.Len());
		}
		return Bytes;
	}

public:
	void StartListening();
	void StopListening();

protected:
	//~ Begin FUdpReceiver delegate
	void OnDataReceived(const FArrayReaderPtr&, const FIPv4Endpoint&);
	//~ End FUdpReceiver delegate

public:
	bool Send(const TSharedRef<TArray<uint8>, ESPMode::ThreadSafe>& Data);

protected:
	FIPv4Endpoint ClientEndpoint;
	FIPv4Endpoint ServerEndpoint;
	
	FSocket* Socket = nullptr;
	ISocketSubsystem* SocketSubSystem;

	TSharedPtr<FUdpSocketSender, ESPMode::ThreadSafe> Sender = nullptr;
	TSharedPtr<FUdpSocketReceiver, ESPMode::ThreadSafe> Receiver = nullptr;
};