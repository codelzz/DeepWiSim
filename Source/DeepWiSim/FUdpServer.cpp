// Fill out your copyright notice in the Description page of Project Settings.


#include "FUdpServer.h"

void FUdpServer::StartListening()
{
	Socket = FUdpSocketBuilder(GetName()).AsReusable();
	TSharedPtr<FInternetAddr, ESPMode::NotThreadSafe> InternetAddr = SocketSubSystem->CreateInternetAddr();
	InternetAddr->SetIp(ServerEndpoint.Address.Value);
	InternetAddr->SetPort(ServerEndpoint.Port);
	Socket->Bind(*InternetAddr);
	Socket->Listen(1);

	Sender = MakeShared<FUdpSocketSender, ESPMode::ThreadSafe>(Socket, TEXT("UdpSender"));
	Receiver = MakeShared<FUdpSocketReceiver, ESPMode::ThreadSafe>(Socket, FTimespan::FromMilliseconds(1.0f), TEXT("UdpReceiver"));
	if (Receiver.IsValid())
	{
		Receiver->OnDataReceived().BindRaw(this, &FUdpServer::OnDataReceived);
		Receiver->Start();
	}
}

void FUdpServer::StopListening()
{
	Socket->Close();
}

void FUdpServer::OnDataReceived(const FArrayReaderPtr& ReaderPtr, const FIPv4Endpoint& Endpoint)
{
	if (Endpoint == ClientEndpoint)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] OnDataReceived"), *GetName());
	}
}

bool FUdpServer::Send(const TSharedRef<TArray<uint8>, ESPMode::ThreadSafe>& Data)
{
	if (!Sender.IsValid()) { return false; }
	
	Sender->Send(Data, ClientEndpoint);
	return false;
}

