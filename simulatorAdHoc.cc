#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/string.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/mobility-model.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/opengym-module.h"
#include "ns3/node-list.h"
using namespace ns3;


//Contador de número de paquetes enviados y recibidos
int recibidos = 0;
int enviados = 0;

Ptr<OpenGymSpace> MyGetObservationSpace(void)
{
  uint32_t numNodos = 6;
  float min = 400.0;
  float max = 800.0;
  std::vector<uint32_t> shape = {numNodos,};
  std::string dtype = TypeNameGet<uint32_t> ();
  Ptr<OpenGymBoxSpace> space = CreateObject<OpenGymBoxSpace> (min, max, shape, dtype);
  NS_LOG_UNCOND ("MyGetObservationSpace: " << space);
  return space;
}

Ptr<OpenGymSpace> MyGetActionSpace(void)
{
  uint32_t numNodos = 6;
  float min = 400.0;
  float max = 800.0;
  std::vector<uint32_t> shape = {numNodos,};
  std::string dtype = TypeNameGet<uint32_t> ();
  Ptr<OpenGymBoxSpace> space = CreateObject<OpenGymBoxSpace> (min, max, shape, dtype);
  NS_LOG_UNCOND ("MyGetActionSpace: " << space);
  return space;
}

bool MyGetGameOver(void)
{
  bool isGameOver = false;
  static float contador = 0.0;
  contador += 1;
  if (contador == 20) {
      isGameOver = true;
  }
  NS_LOG_UNCOND ("Game Over: " << isGameOver);
  return isGameOver;
}

Ptr<OpenGymDataContainer> MyGetObservation(void)
{
  uint32_t numNodos = 6;

  std::vector<uint32_t> shape = {numNodos,};
  Ptr<OpenGymBoxContainer<uint32_t> > box = CreateObject<OpenGymBoxContainer<uint32_t> >(shape);
  uint32_t nodeNum2 = NodeList::GetNNodes ();
    for (uint32_t i=0; i<nodeNum2; i++)
    {
      Ptr<Node> node = NodeList::GetNode(i);
      if (node->GetSystemId() == 0) {

        Ptr<MobilityModel> cpMob = node->GetObject<MobilityModel>();
        Vector m_position = cpMob->GetPosition();
        box->AddValue(m_position.x);
      }
    }
  NS_LOG_UNCOND ("Vector de Observación: " << box);
  return box;
}

float MyGetReward(void)
{
  static float reward = 0;
  if(enviados > 0) {
    reward = ((recibidos*1.0)/(enviados*1.0));
  }
    NS_LOG_UNCOND(reward);
    NS_LOG_UNCOND(recibidos);
  return reward;
}

bool MyExecuteActions(Ptr<OpenGymDataContainer> action)
{
  NS_LOG_UNCOND ("Acciones Ejecutadas: " << action);
  
  Ptr<OpenGymBoxContainer<uint32_t> > box = DynamicCast<OpenGymBoxContainer<uint32_t> >(action);
  std::vector<uint32_t> actionVector = box->GetData();

  uint32_t numNodos = NodeList::GetNNodes ();
  for (uint32_t i=0; i<numNodos; i++)
  {
    Ptr<Node> node = NodeList::GetNode(i);
    if (node->GetSystemId() == 0) {

      Ptr<MobilityModel> cpMob = node->GetObject<MobilityModel>();
      Vector m_position = cpMob->GetPosition();
      m_position.x = actionVector.at(i);
      cpMob->SetPosition(m_position);
    }
  }

  return true;
}

static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize,
                             uint32_t pktCount, Time pktInterval )
{
  enviados += 1;

  socket->Send (Create<Packet> (pktSize));  
}


void ScheduleNextStateRead(double envStepTime, Ptr<OpenGymInterface> openGym, Ptr<Socket> source, uint32_t tamanoPaquete,
                             uint32_t numeroPaquetes, Time interPacketInterval)
{

  GenerateTraffic(source, tamanoPaquete, numeroPaquetes, interPacketInterval);
  GenerateTraffic(source, tamanoPaquete, numeroPaquetes, interPacketInterval);
  GenerateTraffic(source, tamanoPaquete, numeroPaquetes, interPacketInterval);
  GenerateTraffic(source, tamanoPaquete, numeroPaquetes, interPacketInterval);
  GenerateTraffic(source, tamanoPaquete, numeroPaquetes, interPacketInterval);
  GenerateTraffic(source, tamanoPaquete, numeroPaquetes, interPacketInterval);

  Simulator::Schedule (Seconds (envStepTime), &ScheduleNextStateRead, envStepTime, openGym,
                       source, tamanoPaquete, numeroPaquetes, interPacketInterval);
  openGym->NotifyCurrentState();
}

void ReceivePacket (Ptr<Socket> socket)
{
  while (socket->Recv ())
    {
      NS_LOG_UNCOND ("Paquete Recibido!");
      recibidos += 1;
    }
}


int main (int argc, char *argv[])
{
  std::string phyMode ("DsssRate1Mbps");
  double distancia = 5;  // metros
  uint32_t tamanoPaquete = 500; // bytes
  uint32_t numeroPaquetes = 10;
  uint32_t numeroNodos = 10;   
  uint32_t nodoReceptor = 1; 
  uint32_t nodoFuente = 2; 
  double envStepTime = 0.6;
    double interval = 0.1; 
  CommandLine cmd; 
  cmd.AddValue ("Distancia", "Distancia (m)", distancia);
 
  cmd.Parse (argc, argv);
  Time interPacketInterval = Seconds (interval);

  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
                      StringValue (phyMode));


  NodeContainer nodosJerarquicos;
  nodosJerarquicos.Create(6, 0);

  NodeContainer contenedor1;
  contenedor1.Add(nodosJerarquicos.Get(0));
  contenedor1.Add(nodosJerarquicos.Get(1));
  contenedor1.Create (numeroNodos, 1);

  NodeContainer contenedor2;
  contenedor2.Add(nodosJerarquicos.Get(2));
  contenedor2.Add(nodosJerarquicos.Get(3));
  contenedor2.Create (numeroNodos, 2);
  
  NodeContainer contenedor3;
  contenedor3.Add(nodosJerarquicos.Get(4));
  contenedor3.Add(nodosJerarquicos.Get(5));
  contenedor3.Create (numeroNodos, 2);

  WifiHelper wifi;

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  wifiPhy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11);
  
  // Wifi power setted for the simulation 
  wifiPhy.Set ("RxGain", DoubleValue (0) );
  wifiPhy.Set ("TxPowerStart", DoubleValue (33) );
  wifiPhy.Set ("TxPowerEnd", DoubleValue (33) );
  wifiPhy.Set ("TxGain", DoubleValue (0) );
  wifiPhy.Set ("RxSensitivity", DoubleValue (-64) );


  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::TwoRayGroundPropagationLossModel",
  "SystemLoss", DoubleValue(1),
  "HeightAboveZ", DoubleValue(1.5));

  //Wifi channel creation
  wifiPhy.SetChannel (wifiChannel.Create ());
  WifiMacHelper wifiMac;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));
  // Set it to adhoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices1 = wifi.Install (wifiPhy, wifiMac, contenedor1);
  NetDeviceContainer devices2 = wifi.Install (wifiPhy, wifiMac, contenedor2);
  NetDeviceContainer devices3 = wifi.Install (wifiPhy, wifiMac, contenedor3);
  NetDeviceContainer devicesHierar2 = wifi.Install (wifiPhy, wifiMac, nodosJerarquicos);

  // Mobility models creation
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (400),
                                 "MinY", DoubleValue (120),
                                 "DeltaX", DoubleValue (distancia),
                                 "DeltaY", DoubleValue (distancia),
                                 "GridWidth", UintegerValue (5),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  MobilityHelper mobility2;
  mobility2.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (600),
                                 "MinY", DoubleValue (-150),
                                 "DeltaX", DoubleValue (distancia),
                                 "DeltaY", DoubleValue (distancia),
                                 "GridWidth", UintegerValue (5),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  MobilityHelper mobility3;
  mobility3.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (800),
                                 "MinY", DoubleValue (120),
                                 "DeltaX", DoubleValue (distancia),
                                 "DeltaY", DoubleValue (distancia),
                                 "GridWidth", UintegerValue (5),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility3.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  MobilityHelper mobilityHierar2;
  mobilityHierar2.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (600),
                                 "MinY", DoubleValue (0),
                                 "DeltaX", DoubleValue ((1)+3*distancia),
                                 "DeltaY", DoubleValue (distancia),
                                 "GridWidth", UintegerValue (6),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility3.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (contenedor1);
  mobility2.Install (contenedor2);
  mobility3.Install (contenedor3);
  mobilityHierar2.Install (nodosJerarquicos);

  OlsrHelper olsr;
  OlsrHelper olsr2;
  Ipv4StaticRoutingHelper staticRouting;

  Ipv4ListRoutingHelper list;
  list.Add (staticRouting, 0);
  list.Add (olsr, 30);
  InternetStackHelper internet;

  internet.SetRoutingHelper (list);
  internet.Install (contenedor1);
  internet.Install (contenedor2);
  internet.Install (contenedor3);


  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer IP1 = ipv4.Assign (devices1);
  Ipv4InterfaceContainer IP4 = ipv4.Assign (devicesHierar2);
  Ipv4InterfaceContainer IP2 = ipv4.Assign (devices2);
  Ipv4InterfaceContainer IP3 = ipv4.Assign (devices3);

  // Socket definition and package source and objective preparation
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink = Socket::CreateSocket (contenedor1.Get (nodoReceptor), tid);
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
  recvSink->Bind (local);
  recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

  Ptr<Socket> source = Socket::CreateSocket (contenedor3.Get (nodoFuente), tid);
  InetSocketAddress remote = InetSocketAddress (IP1.GetAddress (nodoReceptor, 0), 80);
  source->Connect (remote);

  
  uint32_t openGymPort = 2222;
  Ptr<OpenGymInterface> openGym = CreateObject<OpenGymInterface> (openGymPort);
  openGym->SetGetActionSpaceCb( MakeCallback (&MyGetActionSpace) );
  openGym->SetGetObservationSpaceCb( MakeCallback (&MyGetObservationSpace) );
  openGym->SetGetGameOverCb( MakeCallback (&MyGetGameOver) );
  openGym->SetGetObservationCb( MakeCallback (&MyGetObservation) );
  openGym->SetGetRewardCb( MakeCallback (&MyGetReward) );
  openGym->SetExecuteActionsCb( MakeCallback (&MyExecuteActions) );
  Simulator::Schedule (Seconds(0.0), &ScheduleNextStateRead, envStepTime, openGym, source,
  tamanoPaquete, numeroPaquetes, interPacketInterval);

  NS_LOG_UNCOND ("Prueba desde el nodo " << nodoFuente << " al nodo " << nodoReceptor << " con distancia de  " << distancia);

  Simulator::Run ();
  Simulator::Stop (Seconds (100));
  Simulator::Destroy ();

  return 0;
}

