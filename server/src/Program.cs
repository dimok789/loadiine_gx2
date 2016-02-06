using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Collections.Generic;

namespace cafiine_server
{
    class Program
    {
        // Command bytes
        // com
        public const byte BYTE_NORMAL = 0xff;
        public const byte BYTE_SPECIAL = 0xfe;
        public const byte BYTE_OK = 0xfd;
        public const byte BYTE_PING = 0xfc;
        public const byte BYTE_LOG_STR = 0xfb;
        public const byte BYTE_DISCONNECT = 0xfa;

        // sd
        public const byte BYTE_MOUNT_SD = 0xe0;
        public const byte BYTE_MOUNT_SD_OK = 0xe1;
        public const byte BYTE_MOUNT_SD_BAD = 0xe2;

        // replacement
        public const byte BYTE_STAT = 0x00;
        public const byte BYTE_STAT_ASYNC = 0x01;
        public const byte BYTE_OPEN_FILE = 0x02;
        public const byte BYTE_OPEN_FILE_ASYNC = 0x03;
        public const byte BYTE_OPEN_DIR = 0x04;
        public const byte BYTE_OPEN_DIR_ASYNC = 0x05;
        public const byte BYTE_CHANGE_DIR = 0x06;
        public const byte BYTE_CHANGE_DIR_ASYNC = 0x07;
        public const byte BYTE_MAKE_DIR = 0x08;
        public const byte BYTE_MAKE_DIR_ASYNC = 0x09;
        public const byte BYTE_RENAME = 0x0a;
        public const byte BYTE_RENAME_ASYNC = 0x0b;
        public const byte BYTE_REMOVE = 0x0c;
        public const byte BYTE_REMOVE_ASYNC = 0x0d;

        // log
        public const byte BYTE_CLOSE_FILE = 0x40;
        public const byte BYTE_CLOSE_FILE_ASYNC = 0x41;
        public const byte BYTE_CLOSE_DIR = 0x42;
        public const byte BYTE_CLOSE_DIR_ASYNC = 0x43;
        public const byte BYTE_FLUSH_FILE = 0x44;
        public const byte BYTE_GET_ERROR_CODE_FOR_VIEWER = 0x45;
        public const byte BYTE_GET_LAST_ERROR = 0x46;
        public const byte BYTE_GET_MOUNT_SOURCE = 0x47;
        public const byte BYTE_GET_MOUNT_SOURCE_NEXT = 0x48;
        public const byte BYTE_GET_POS_FILE = 0x49;
        public const byte BYTE_SET_POS_FILE = 0x4A;
        public const byte BYTE_GET_STAT_FILE = 0x4B;
        public const byte BYTE_EOF = 0x4C;
        public const byte BYTE_READ_FILE = 0x4D;
        public const byte BYTE_READ_FILE_ASYNC = 0x4E;
        public const byte BYTE_READ_FILE_WITH_POS = 0x4F;
        public const byte BYTE_READ_DIR = 0x50;
        public const byte BYTE_READ_DIR_ASYNC = 0x51;
        public const byte BYTE_GET_CWD = 0x52;
        public const byte BYTE_SET_STATE_CHG_NOTIF = 0x53;
        public const byte BYTE_TRUNCATE_FILE = 0x54;
        public const byte BYTE_WRITE_FILE = 0x55;
        public const byte BYTE_WRITE_FILE_WITH_POS = 0x56;

        public const byte BYTE_CREATE_THREAD = 0x60;

        // Other defines
        public const int FS_MAX_ENTNAME_SIZE = 256;
        public const int FS_MAX_ENTNAME_SIZE_PAD = 0;

        public const int FS_MAX_LOCALPATH_SIZE = 511;
        public const int FS_MAX_MOUNTPATH_SIZE = 128;

        // Logs folder
        public static string logs_root = "logs";
        
        static void Main(string[] args)
        {
            // Check if logs folder
            if (!Directory.Exists(logs_root))
            {
                Console.Error.WriteLine("Logs directory `{0}' does not exist!", logs_root);
                return;
            }
            // Delete logs
            System.IO.DirectoryInfo downloadedMessageInfo = new DirectoryInfo(logs_root);
            foreach (FileInfo file in downloadedMessageInfo.GetFiles())
            {
                file.Delete();
            }

            // Start server
            string name = "[listener]";
            try
            {
                TcpListener listener = new TcpListener(IPAddress.Any, 7332);
                listener.Start();
                Console.WriteLine(name + " Listening on 7332");

                int index = 0;
                while (true)
                {
                    TcpClient client = listener.AcceptTcpClient();
                    Console.WriteLine("connected");
                    Thread thread = new Thread(Handle);
                    thread.Name = "[" + index.ToString() + "]";
                    thread.Start(client);
                    index++;
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(name + " " + e.Message);
            }
            Console.WriteLine(name + " Exit");
        }

        static void Log(StreamWriter log, String str)
        {
            log.WriteLine(str);
            log.Flush();
            Console.WriteLine(str);
        }

        static void Handle(object client_obj)
        {
            string name = Thread.CurrentThread.Name;
            StreamWriter log = null;

            try
            {
                TcpClient client = (TcpClient)client_obj;
                using (NetworkStream stream = client.GetStream())
                {
                    EndianBinaryReader reader = new EndianBinaryReader(stream);
                    EndianBinaryWriter writer = new EndianBinaryWriter(stream);


                    // Log connection
                    Console.WriteLine(name + " Accepted connection from client " + client.Client.RemoteEndPoint.ToString());

                    // Create log file for current thread
                    log = new StreamWriter(logs_root + "\\" + DateTime.Now.ToString("yyyy-MM-dd") + "-" + name + ".txt");
                    log.WriteLine(name + " Accepted connection from client " + client.Client.RemoteEndPoint.ToString());

                    writer.Write(BYTE_SPECIAL);

                    while (true)
                    {
                        byte cmd_byte = reader.ReadByte();
                        switch (cmd_byte)
                        {
                            // cmd
                            case BYTE_PING:
                                {
                                    int val1 = reader.ReadInt32();
                                    int val2 = reader.ReadInt32();

                                    Log(log, name + " PING RECEIVED with values : " + val1.ToString() + " - " + val2.ToString());
                                    break;
                                }
                            case BYTE_LOG_STR:
                                {
                                    int len_str = reader.ReadInt32();
                                    string str = reader.ReadString(Encoding.ASCII, len_str - 1);
                                    if (reader.ReadByte() != 0) throw new InvalidDataException();

                                    Log(log, name + " LogString =>(\"" + str + "\")");
                                    break;
                                }
                            case BYTE_DISCONNECT:
                                {
                                    Log(log, name + " DISCONNECT");
                                    break;
                                }

                            // sd
                            case BYTE_MOUNT_SD:
                                {
                                    Log(log, name + " Trying to mount SD card");
                                    break;
                                }
                            case BYTE_MOUNT_SD_OK:
                                {
                                    Log(log, name + " SD card mounted !");
                                    break;
                                }
                            case BYTE_MOUNT_SD_BAD:
                                {
                                    Log(log, name + " Can't mount SD card");
                                    break;
                                }

                            // replacement
                            case BYTE_STAT:
                            case BYTE_STAT_ASYNC:
                                {
                                    int len_path = reader.ReadInt32();
                                    string path = reader.ReadString(Encoding.ASCII, len_path - 1);
                                    if (reader.ReadByte() != 0) throw new InvalidDataException();

                                    if (cmd_byte == BYTE_STAT)
                                        Log(log, name + " FSGetStat(\"" + path + "\")");
                                    else
                                        Log(log, name + " FSGetStatAsync(\"" + path + "\")");
                                    break;
                                }
                            case BYTE_OPEN_FILE:
                            case BYTE_OPEN_FILE_ASYNC:
                                {
                                    int len_str = reader.ReadInt32();
                                    string str = reader.ReadString(Encoding.ASCII, len_str - 1);
                                    if (reader.ReadByte() != 0) throw new InvalidDataException();

                                    if (cmd_byte == BYTE_OPEN_FILE)
                                        Log(log, name + " FSOpenFile(\"" + str + "\")");
                                    else
                                        Log(log, name + " FSOpenFileAsync(\"" + str + "\")");

                                    break;
                                }
                            case BYTE_OPEN_DIR:
                            case BYTE_OPEN_DIR_ASYNC:
                                {
                                    int len_path = reader.ReadInt32();
                                    string path = reader.ReadString(Encoding.ASCII, len_path - 1);
                                    if (reader.ReadByte() != 0) throw new InvalidDataException();

                                    if (cmd_byte == BYTE_OPEN_DIR)
                                        Log(log, name + " FSOpenDir(\"" + path + "\")");
                                    else
                                        Log(log, name + " FSOpenDirAsync(\"" + path + "\")");

                                    break;
                                }
                            case BYTE_CHANGE_DIR:
                            case BYTE_CHANGE_DIR_ASYNC:
                                {
                                    int len_path = reader.ReadInt32();
                                    string path = reader.ReadString(Encoding.ASCII, len_path - 1);
                                    if (reader.ReadByte() != 0) throw new InvalidDataException();

                                    if (cmd_byte == BYTE_CHANGE_DIR)
                                        Log(log, name + " FSChangeDir(\"" + path + "\")");
                                    else
                                        Log(log, name + " FSChangeDirAsync(\"" + path + "\")");

                                    break;
                                }
                            case BYTE_MAKE_DIR:
                            case BYTE_MAKE_DIR_ASYNC:
                                {
                                    int len_path = reader.ReadInt32();
                                    string path = reader.ReadString(Encoding.ASCII, len_path - 1);
                                    if (reader.ReadByte() != 0) throw new InvalidDataException();

                                    if (cmd_byte == BYTE_CHANGE_DIR)
                                        Log(log, name + " FSMakeDir(\"" + path + "\")");
                                    else
                                        Log(log, name + " FSMakeDirAsync(\"" + path + "\")");

                                    break;
                                }
                            case BYTE_RENAME:
                            case BYTE_RENAME_ASYNC:
                                {
                                    int len_path = reader.ReadInt32();
                                    string path = reader.ReadString(Encoding.ASCII, len_path - 1);
                                    if (reader.ReadByte() != 0) throw new InvalidDataException();

                                    if (cmd_byte == BYTE_CHANGE_DIR)
                                        Log(log, name + " FSRename(\"" + path + "\")");
                                    else
                                        Log(log, name + " FSRenameAsync(\"" + path + "\")");

                                    break;
                                }
                            case BYTE_REMOVE:
                            case BYTE_REMOVE_ASYNC:
                                {
                                    int len_path = reader.ReadInt32();
                                    string path = reader.ReadString(Encoding.ASCII, len_path - 1);
                                    if (reader.ReadByte() != 0) throw new InvalidDataException();

                                    if (cmd_byte == BYTE_CHANGE_DIR)
                                        Log(log, name + " FSRemove(\"" + path + "\")");
                                    else
                                        Log(log, name + " FSRemoveAsync(\"" + path + "\")");

                                    break;
                                }

                            // Log
                            case BYTE_CLOSE_FILE:
                                {
                                     Log(log, name + " FSCloseFile()");
                                     break;
                                }
                            case BYTE_CLOSE_FILE_ASYNC:
                                {
                                    Log(log, name + " FSCloseFileAsync()");
                                    break;
                                }
                            case BYTE_CLOSE_DIR:
                                {
                                    Log(log, name + " FSCloseDir()");
                                    break;
                                }
                            case BYTE_CLOSE_DIR_ASYNC:
                                {
                                    Log(log, name + " FSCloseDirAsync()");
                                    break;
                                }
                            case BYTE_FLUSH_FILE:
                                {
                                    Log(log, name + " FSFlushFile()");
                                    break;
                                }
                            case BYTE_GET_ERROR_CODE_FOR_VIEWER:
                                {
                                    Log(log, name + " FSGetErrorCodeForViewer()");
                                    break;
                                }
                            case BYTE_GET_LAST_ERROR:
                                {
                                    Log(log, name + " FSGetLastError()");
                                    break;
                                }
                            case BYTE_GET_MOUNT_SOURCE:
                                {
                                    Log(log, name + " FsGetMountSource()");
                                    break;
                                }
                            case BYTE_GET_MOUNT_SOURCE_NEXT:
                                {
                                    Log(log, name + " FsGetMountSourceNext()");
                                    break;
                                }
                            case BYTE_GET_POS_FILE:
                                {
                                    Log(log, name + " FSGetPos()");
                                    break;
                                }
                            case BYTE_SET_POS_FILE:
                                {
                                    Log(log, name + " FSSetPos()");
                                    break;
                                }
                            case BYTE_GET_STAT_FILE:
                                {
                                    Log(log, name + " FSGetStatFile()");
                                    break;
                                }
                            case BYTE_EOF:
                                {
                                    Log(log, name + " FSGetEof()");
                                    break;
                                }
                            case BYTE_READ_FILE:
                                {
                                    Log(log, name + " FSReadFile()");
                                    break;
                                }
                            case BYTE_READ_FILE_ASYNC:
                                {
                                    Log(log, name + " FSReadFileAsync()");
                                    break;
                                }
                            case BYTE_READ_FILE_WITH_POS:
                                {
                                    Log(log, name + " FSReadFileWithPos()");
                                    break;
                                }
                            case BYTE_READ_DIR:
                                {
                                    Log(log, name + " FSReadDir()");
                                    break;
                                }
                            case BYTE_READ_DIR_ASYNC:
                                {
                                    Log(log, name + " FSReadDirAsync()");
                                    break;
                                }
                            case BYTE_GET_CWD:
                                {
                                    Log(log, name + " FSGetCwd()");
                                    break;
                                }
                            case BYTE_SET_STATE_CHG_NOTIF:
                                {
                                    Log(log, name + " FSSetStateChangeNotification()");
                                    break;
                                }
                            case BYTE_TRUNCATE_FILE:
                                {
                                    Log(log, name + " FSTruncateFile()");
                                    break;
                                }
                            case BYTE_WRITE_FILE:
                                {
                                    Log(log, name + " FSWriteFile()");
                                    break;
                                }
                            case BYTE_WRITE_FILE_WITH_POS:
                                {
                                    Log(log, name + " FSWriteFileWithPos()");
                                    break;
                                }


                            case BYTE_CREATE_THREAD:
                                {
                                    Log(log, name + " CreateThread()");
                                    break;
                                }

                            default:
                                throw new InvalidDataException();
                        }
                    }
                }
            }
            catch (Exception e)
            {
                if (log != null)
                    Log(log, name + " " + e.Message);
                else
                    Console.WriteLine(name + " " + e.Message);
            }
            finally
            {
                if (log != null)
                    log.Close();
            }
            Console.WriteLine(name + " Exit");
        }
    }
}
