// MainWindow.xaml.cs
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Threading;
using Google.Protobuf;

namespace dummyClient
{
    public partial class MainWindow : Window
    {
        private List<DummyClient> clients = new();
        private DummyClient singleClient;
        private System.Timers.Timer dummyChatTimer;
        private System.Timers.Timer singleChatTimer;
        private int dummyMessageCount = 0;
        private int singleMessageCount = 0;
        private int broadcastIndex = 0;
        private ConcurrentQueue<string> _logQueue = new();
        private DispatcherTimer _logFlushTimer;

        public MainWindow()
        {
            InitializeComponent();

            dummyChatTimer = new System.Timers.Timer(100);
            dummyChatTimer.Elapsed += (s, e) => Dispatcher.Invoke(() => BroadcastDummyChat());

            singleChatTimer = new System.Timers.Timer(2000);
            singleChatTimer.Elapsed += (s, e) => Dispatcher.Invoke(() => SendSingleChat());

            _logFlushTimer = new DispatcherTimer
            {
                Interval = TimeSpan.FromMilliseconds(100)
            };
            _logFlushTimer.Tick += (s, e) =>
            {
                while (_logQueue.TryDequeue(out var line))
                {
                    txtLog.AppendText(line + "\n");
                }
            };
            _logFlushTimer.Start();
        }

        private void BtnDummyConnect_Click(object sender, RoutedEventArgs e)
        {
            if (!int.TryParse(txtDummyCount.Text, out int count)) return;

            for (int i = 0; i < count; i++)
            {
                var client = new DummyClient(LogPacket);
                if (client.Connect())
                {
                    clients.Add(client);
                    Log($"Dummy {clients.Count - 1} connected");
                }
            }
        }

        private void BtnDummyDisconnect_Click(object sender, RoutedEventArgs e)
        {
            foreach (var client in clients)
                client.Disconnect();

            Log($"{clients.Count} dummy clients disconnected");
            clients.Clear();
        }

        private void BtnDummyChatStart_Click(object sender, RoutedEventArgs e)
        {
            if (dummyChatTimer != null)
            {
                dummyChatTimer.Start();
                Log("Dummy chat started");
            }
        }

        private void BtnDummyChatStop_Click(object sender, RoutedEventArgs e)
        {
            if (dummyChatTimer != null)
            {
                dummyChatTimer.Stop();
                Log("Dummy chat stopped");
            }
        }

        private void BroadcastDummyChat()
        {
            if (clients.Count == 0) return;

            if (broadcastIndex >= clients.Count)
                broadcastIndex = 0;

            var client = clients[broadcastIndex];
            client.SendChatPacket($"Dummy msg {++dummyMessageCount}");

            broadcastIndex = (broadcastIndex + 1) % clients.Count;
        }

        private void BtnSingleConnect_Click(object sender, RoutedEventArgs e)
        {
            if (singleClient == null)
            {
                singleClient = new DummyClient(LogPacket);
                if (singleClient.Connect())
                {
                    Log("Single client connected");
                }
            }
        }

        private void BtnSingleDisconnect_Click(object sender, RoutedEventArgs e)
        {
            if (singleClient != null)
            {
                singleClient.Disconnect();
                singleClient = null;
                Log("Single client disconnected");
            }
        }

        private void BtnSingleChat_Click(object sender, RoutedEventArgs e)
        {
            if (singleChatTimer != null)
            {
                singleChatTimer.Start();
                Log("Single chat started");
            }
        }

        private void BtnSingleChatStop_Click(object sender, RoutedEventArgs e)
        {
            if (singleChatTimer != null)
            {
                singleChatTimer.Stop();
                Log("Single chat stopped");
            }
        }

        private void SendSingleChat()
        {
            singleClient?.SendChatPacket($"Single msg {++singleMessageCount}");
        }

        private void Log(string msg)
        {
            _logQueue.Enqueue($"[{DateTime.Now:HH:mm:ss}] {msg}");
        }

        private void LogPacket(string msg)
        {
            Dispatcher.Invoke(() =>
            {
                txtPacketLog.AppendText($"[{DateTime.Now:HH:mm:ss}] {msg}\n");
                txtPacketLog.ScrollToEnd();
            });
        }
    }

    public class DummyClient
    {
        private TcpClient tcpClient;
        private NetworkStream stream;
        private Thread recvThread;
        private bool running = false;
        private static uint packetId = 1;
        private readonly Action<string> logAction;
        private enum DisconnectReason { None, Local, Remote }
        private DisconnectReason disconnectReason = DisconnectReason.None;

        public DummyClient(Action<string> log)
        {
            logAction = log;
        }

        public bool Connect()
        {
            try
            {
                tcpClient = new TcpClient("127.0.0.1", 8000);
                stream = tcpClient.GetStream();
                running = true;
                StartReceiveLoop();
                return true;
            }
            catch
            {
                return false;
            }
        }

        public void Disconnect()
        {
            disconnectReason = DisconnectReason.Local;
            running = false;
            stream?.Close();
            tcpClient?.Close();
        }

        public void SendChatPacket(string message)
        {
            var chat = new ChatPacket
            {
                Header = new PacketHeader
                {
                    Type = PacketType.Chat,
                    PacketId = packetId++,
                    Length = 0
                },
                SessionId = 0,
                Message = message,
            };

            using var ms = new MemoryStream();
            chat.WriteTo(ms);
            byte[] body = ms.ToArray();
            ushort len = (ushort)body.Length;

            using var final = new MemoryStream();
            final.Write(BitConverter.GetBytes(IPAddress.HostToNetworkOrder((short)len)));
            final.Write(body);

            try
            {
                stream.Write(final.ToArray(), 0, (int)final.Length);
            }
            catch (Exception ex)
            {
                logAction?.Invoke($"Send failed: {ex.Message}");
            }
        }

        private void StartReceiveLoop()
        {
            recvThread = new Thread(async () =>
            {
                var buffer = new byte[4096];
                try
                {
                    while (running)
                    {
                        if (!await ReadExactAsync(buffer, 0, 2)) break;

                        short len = IPAddress.NetworkToHostOrder(BitConverter.ToInt16(buffer, 0));
                        if (len <= 0 || len > buffer.Length - 2) break;
                        if (!await ReadExactAsync(buffer, 2, len)) break;

                        try
                        {
                            var chatPacket = ChatPacket.Parser.ParseFrom(buffer, 2, len);
                            var type = chatPacket.Header?.Type ?? PacketType.None;

                            switch (type)
                            {
                                case PacketType.Chat:
                                    logAction?.Invoke($"[CHAT] {chatPacket.SessionId}: {chatPacket.Message}");
                                    break;
                                case PacketType.Waiting:
                                    var waitPacket = WaitingPacket.Parser.ParseFrom(buffer, 2, len);
                                    logAction?.Invoke($"[WAIT] {waitPacket.WaitingNumber}: {waitPacket.Message}");
                                    break;
                                default:
                                    logAction?.Invoke($"[UNKNOWN] Header type: {type}");
                                    break;
                            }
                        }
                        catch (Exception ex)
                        {
                            logAction?.Invoke($"[ERROR] 패킷 파싱 실패: {ex.Message}");
                        }

                    }
                }
                catch (Exception ex)
                {
                    if (ex is IOException ioEx && ioEx.InnerException is SocketException sockEx)
                    {
                        if (sockEx.SocketErrorCode == SocketError.ConnectionReset)
                        {
                            logAction?.Invoke("[INFO] 수신 종료: 서버에서 연결을 강제로 종료했습니다.");
                        }
                        else if (sockEx.SocketErrorCode == SocketError.OperationAborted)
                        {
                            logAction?.Invoke("[INFO] 수신 종료: 클라이언트에서 정상적으로 연결을 종료했습니다.");
                        }
                        else
                        {
                            logAction?.Invoke($"[ERROR] Recv socket error: {sockEx.SocketErrorCode} - {sockEx.Message}");
                        }
                    }
                    else
                    {
                        logAction?.Invoke($"[ERROR] Recv exception: {ex.Message}");
                    }
                }

            });
            recvThread.IsBackground = true;
            recvThread.Start();
        }

        private async Task<bool> ReadExactAsync(byte[] buf, int offset, int size)
        {
            int total = 0;
            while (total < size)
            {
                int read = await stream.ReadAsync(buf, offset + total, size - total);
                if (read <= 0) return false;
                total += read;
            }
            return true;
        }
    }
}
