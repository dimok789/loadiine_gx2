using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace System.IO
{
    sealed class EndianBinaryWriter : IDisposable
    {
        private bool disposed;
        private byte[] buffer;

        private delegate void ArrayReverse(byte[] array, int count);
        private static readonly ArrayReverse[] fastReverse = new ArrayReverse[] { null, null, ArrayReverse2, null, ArrayReverse4, null, null, null, ArrayReverse8 };

        private static Dictionary<Type, List<Tuple<int, TypeCode>>> parserCache = new Dictionary<Type, List<Tuple<int, TypeCode>>>();

        public Stream BaseStream { get; private set; }
        public Endianness Endianness { get; set; }
        public static Endianness SystemEndianness { get { return BitConverter.IsLittleEndian ? Endianness.LittleEndian : Endianness.BigEndian; } }

        private bool Reverse { get { return SystemEndianness != Endianness; } }

        public EndianBinaryWriter(Stream baseStream)
            : this(baseStream, Endianness.BigEndian)
        { }

        public EndianBinaryWriter(Stream baseStream, Endianness endianness)
        {
            if (baseStream == null) throw new ArgumentNullException("baseStream");
            if (!baseStream.CanWrite) throw new ArgumentException("base stream is not writeable", "baseStream");

            BaseStream = baseStream;
            Endianness = endianness;
        }

        ~EndianBinaryWriter()
        {
            Dispose(false);
        }

        private void WriteBuffer(int bytes, int stride)
        {
            if (Reverse)
            {
                if (fastReverse[stride] != null)
                    fastReverse[stride](buffer, bytes);
                else
                    for (int i = 0; i < bytes; i += stride)
                    {
                        Array.Reverse(buffer, i, stride);
                    }
            }

            BaseStream.Write(buffer, 0, bytes);
        }

        private static void ArrayReverse2(byte[] array, int arrayLength)
        {
            byte temp;

            while (arrayLength > 0)
            {
                temp = array[arrayLength - 2];
                array[arrayLength - 2] = array[arrayLength - 1];
                array[arrayLength - 1] = temp;
                arrayLength -= 2;
            }
        }

        private static void ArrayReverse4(byte[] array, int arrayLength)
        {
            byte temp;

            while (arrayLength > 0)
            {
                temp = array[arrayLength - 3];
                array[arrayLength - 3] = array[arrayLength - 2];
                array[arrayLength - 2] = temp;
                temp = array[arrayLength - 4];
                array[arrayLength - 4] = array[arrayLength - 1];
                array[arrayLength - 1] = temp;
                arrayLength -= 4;
            }
        }

        private static void ArrayReverse8(byte[] array, int arrayLength)
        {
            byte temp;

            while (arrayLength > 0)
            {
                temp = array[arrayLength - 5];
                array[arrayLength - 5] = array[arrayLength - 4];
                array[arrayLength - 4] = temp;
                temp = array[arrayLength - 6];
                array[arrayLength - 6] = array[arrayLength - 3];
                array[arrayLength - 3] = temp;
                temp = array[arrayLength - 7];
                array[arrayLength - 7] = array[arrayLength - 2];
                array[arrayLength - 2] = temp;
                temp = array[arrayLength - 8];
                array[arrayLength - 8] = array[arrayLength - 1];
                array[arrayLength - 1] = temp;
                arrayLength -= 8;
            }
        }

        private void CreateBuffer(int size)
        {
            if (buffer == null || buffer.Length < size)
                buffer = new byte[size];
        }

        public void Write(byte value)
        {
            CreateBuffer(1);
            buffer[0] = value;
            WriteBuffer(1, 1);
        }

        public void Write(byte[] value, int offset, int count)
        {
            CreateBuffer(count);
            Array.Copy(value, offset, buffer, 0, count);
            WriteBuffer(count, 1);
        }

        public void Write(sbyte value)
        {
            CreateBuffer(1);
            unchecked
            {
                buffer[0] = (byte)value;
            }
            WriteBuffer(1, 1);
        }

        public void Write(sbyte[] value, int offset, int count)
        {
            CreateBuffer(count);

            unchecked
            {
                for (int i = 0; i < count; i++)
                {
                    buffer[i] = (byte)value[i + offset];
                }
            }

            WriteBuffer(count, 1);
        }

        public void Write(char value, Encoding encoding)
        {
            int size;

            if (encoding == null) throw new ArgumentNullException("encoding");

            size = GetEncodingSize(encoding);
            CreateBuffer(size);
            Array.Copy(encoding.GetBytes(new string(value, 1)), 0, buffer, 0, size);
            WriteBuffer(size, size);
        }

        public void Write(char[] value, int offset, int count, Encoding encoding)
        {
            int size;

            if (encoding == null) throw new ArgumentNullException("encoding");

            size = GetEncodingSize(encoding);
            CreateBuffer(size * count);
            Array.Copy(encoding.GetBytes(value, offset, count), 0, buffer, 0, count * size);
            WriteBuffer(size * count, size);
        }

        private static int GetEncodingSize(Encoding encoding)
        {
            if (encoding == Encoding.UTF8 || encoding == Encoding.ASCII)
                return 1;
            else if (encoding == Encoding.Unicode || encoding == Encoding.BigEndianUnicode)
                return 2;

            return 1;
        }

        public void Write(string value,Encoding encoding,  bool nullTerminated)
        {
            Write(value.ToCharArray(), 0, value.Length, encoding);
            if (nullTerminated)
                Write('\0', encoding);
        }

        public void Write(double value)
        {
            const int size = sizeof(double);

            CreateBuffer(size);
            Array.Copy(BitConverter.GetBytes(value), 0, buffer, 0, size);
            WriteBuffer(size, size);
        }

        public void Write(double[] value, int offset, int count)
        {
            const int size = sizeof(double);

            CreateBuffer(size * count);
            for (int i = 0; i < count; i++)
            {
                Array.Copy(BitConverter.GetBytes(value[i + offset]), 0, buffer, i * size, size);
            }

            WriteBuffer(size * count, size);
        }

        public void Write(Single value)
        {
            const int size = sizeof(Single);

            CreateBuffer(size);
            Array.Copy(BitConverter.GetBytes(value), 0, buffer, 0, size);
            WriteBuffer(size, size);
        }

        public void Write(Single[] value, int offset, int count)
        {
            const int size = sizeof(Single);

            CreateBuffer(size * count);
            for (int i = 0; i < count; i++)
            {
                Array.Copy(BitConverter.GetBytes(value[i + offset]), 0, buffer, i * size, size);
            }

            WriteBuffer(size * count, size);
        }

        public void Write(Int32 value)
        {
            const int size = sizeof(Int32);

            CreateBuffer(size);
            Array.Copy(BitConverter.GetBytes(value), 0, buffer, 0, size);
            WriteBuffer(size, size);
        }

        public void Write(Int32[] value, int offset, int count)
        {
            const int size = sizeof(Int32);

            CreateBuffer(size * count);
            for (int i = 0; i < count; i++)
            {
                Array.Copy(BitConverter.GetBytes(value[i + offset]), 0, buffer, i * size, size);
            }

            WriteBuffer(size * count, size);
        }

        public void Write(Int64 value)
        {
            const int size = sizeof(Int64);

            CreateBuffer(size);
            Array.Copy(BitConverter.GetBytes(value), 0, buffer, 0, size);
            WriteBuffer(size, size);
        }

        public void Write(Int64[] value, int offset, int count)
        {
            const int size = sizeof(Int64);

            CreateBuffer(size * count);
            for (int i = 0; i < count; i++)
            {
                Array.Copy(BitConverter.GetBytes(value[i + offset]), 0, buffer, i * size, size);
            }

            WriteBuffer(size * count, size);
        }

        public void Write(Int16 value)
        {
            const int size = sizeof(Int16);

            CreateBuffer(size);
            Array.Copy(BitConverter.GetBytes(value), 0, buffer, 0, size);
            WriteBuffer(size, size);
        }

        public void Write(Int16[] value, int offset, int count)
        {
            const int size = sizeof(Int16);

            CreateBuffer(size * count);
            for (int i = 0; i < count; i++)
            {
                Array.Copy(BitConverter.GetBytes(value[i + offset]), 0, buffer, i * size, size);
            }

            WriteBuffer(size * count, size);
        }

        public void Write(UInt16 value)
        {
            const int size = sizeof(UInt16);

            CreateBuffer(size);
            Array.Copy(BitConverter.GetBytes(value), 0, buffer, 0, size);
            WriteBuffer(size, size);
        }

        public void Write(UInt16[] value, int offset, int count)
        {
            const int size = sizeof(UInt16);

            CreateBuffer(size * count);
            for (int i = 0; i < count; i++)
            {
                Array.Copy(BitConverter.GetBytes(value[i + offset]), 0, buffer, i * size, size);
            }

            WriteBuffer(size * count, size);
        }

        public void Write(UInt32 value)
        {
            const int size = sizeof(UInt32);

            CreateBuffer(size);
            Array.Copy(BitConverter.GetBytes(value), 0, buffer, 0, size);
            WriteBuffer(size, size);
        }

        public void Write(UInt32[] value, int offset, int count)
        {
            const int size = sizeof(UInt32);

            CreateBuffer(size * count);
            for (int i = 0; i < count; i++)
            {
                Array.Copy(BitConverter.GetBytes(value[i + offset]), 0, buffer, i * size, size);
            }

            WriteBuffer(size * count, size);
        }

        public void Write(UInt64 value)
        {
            const int size = sizeof(UInt64);

            CreateBuffer(size);
            Array.Copy(BitConverter.GetBytes(value), 0, buffer, 0, size);
            WriteBuffer(size, size);
        }

        public void Write(UInt64[] value, int offset, int count)
        {
            const int size = sizeof(UInt64);

            CreateBuffer(size * count);
            for (int i = 0; i < count; i++)
            {
                Array.Copy(BitConverter.GetBytes(value[i + offset]), 0, buffer, i * size, size);
            }

            WriteBuffer(size * count, size);
        }

        public void WritePadding(int multiple, byte padding)
        {
            int length = (int)(BaseStream.Position % multiple);

            if (length != 0)
                while (length != multiple)
                {
                    BaseStream.WriteByte(padding);
                    length++;
                }
        }

        public void WritePadding(int multiple, byte padding, long from, int offset)
        {
            int length = (int)((BaseStream.Position - from) % multiple);
            length = (length + offset) % multiple;

            if (length != 0)
                while (length != multiple)
                {
                    BaseStream.WriteByte(padding);
                    length++;
                }
        }

        private List<Tuple<int, TypeCode>> GetParser(Type type)
        {
            List<Tuple<int, TypeCode>> parser;

            /* A parser describes how to read in a type in an Endian
             * appropriate manner. Basically it describes as series of calls to
             * the Read* methods above to parse the structure.
             * The parser runs through each element in the list in order. If
             * the TypeCode is not Empty then it reads an array of values
             * according to the integer. Otherwise it skips a number of bytes. */

            try
            {
                parser = parserCache[type];
            }
            catch (KeyNotFoundException)
            {
                parser = new List<Tuple<int, TypeCode>>();

                if (Endianness != SystemEndianness)
                {
                    int pos, sz;

                    pos = 0;
                    foreach (var item in type.GetFields())
                    {
                        int off = Marshal.OffsetOf(type, item.Name).ToInt32();
                        if (off != pos)
                        {
                            parser.Add(new Tuple<int, TypeCode>(off - pos, TypeCode.Empty));
                            pos = off;
                        }
                        switch (Type.GetTypeCode(item.FieldType))
                        {
                            case TypeCode.Byte:
                            case TypeCode.SByte:
                                pos += 1;
                                parser.Add(new Tuple<int, TypeCode>(1, Type.GetTypeCode(item.FieldType)));
                                break;
                            case TypeCode.Int16:
                            case TypeCode.UInt16:
                                pos += 2;
                                parser.Add(new Tuple<int, TypeCode>(1, Type.GetTypeCode(item.FieldType)));
                                break;
                            case TypeCode.Int32:
                            case TypeCode.UInt32:
                            case TypeCode.Single:
                                pos += 4;
                                parser.Add(new Tuple<int, TypeCode>(1, Type.GetTypeCode(item.FieldType)));
                                break;
                            case TypeCode.Int64:
                            case TypeCode.UInt64:
                            case TypeCode.Double:
                                pos += 8;
                                parser.Add(new Tuple<int, TypeCode>(1, Type.GetTypeCode(item.FieldType)));
                                break;
                            case TypeCode.Object:
                                if (item.FieldType.IsArray)
                                {
                                    /* array */
                                    Type elementType;
                                    MarshalAsAttribute[] attrs;
                                    MarshalAsAttribute attr;

                                    attrs = (MarshalAsAttribute[])item.GetCustomAttributes(typeof(MarshalAsAttribute), false);
                                    if (attrs.Length != 1)
                                        throw new ArgumentException(String.Format("Field `{0}' is an array without a MarshalAs attribute.", item.Name), "type");

                                    attr = attrs[0];
                                    if (attr.Value != UnmanagedType.ByValArray)
                                        throw new ArgumentException(String.Format("Field `{0}' is not a ByValArray.", item.Name), "type");

                                    elementType = item.FieldType.GetElementType();
                                    switch (Type.GetTypeCode(elementType))
                                    {
                                        case TypeCode.Byte:
                                        case TypeCode.SByte:
                                            pos += 1 * attr.SizeConst;
                                            parser.Add(new Tuple<int, TypeCode>(attr.SizeConst, Type.GetTypeCode(elementType)));
                                            break;
                                        case TypeCode.Int16:
                                        case TypeCode.UInt16:
                                            pos += 2 * attr.SizeConst;
                                            parser.Add(new Tuple<int, TypeCode>(attr.SizeConst, Type.GetTypeCode(elementType)));
                                            break;
                                        case TypeCode.Int32:
                                        case TypeCode.UInt32:
                                        case TypeCode.Single:
                                            pos += 4 * attr.SizeConst;
                                            parser.Add(new Tuple<int, TypeCode>(attr.SizeConst, Type.GetTypeCode(elementType)));
                                            break;
                                        case TypeCode.Int64:
                                        case TypeCode.UInt64:
                                        case TypeCode.Double:
                                            pos += 8 * attr.SizeConst;
                                            parser.Add(new Tuple<int, TypeCode>(attr.SizeConst, Type.GetTypeCode(elementType)));
                                            break;
                                        case TypeCode.Object:
                                            /* nested structure */
                                            for (int i = 0; i < attr.SizeConst; i++)
                                            {
                                                pos += Marshal.SizeOf(elementType);
                                                parser.AddRange(GetParser(elementType));
                                            }
                                            break;
                                        default:
                                            break;
                                    }
                                }
                                else
                                {
                                    /* nested structure */
                                    pos += Marshal.SizeOf(item.FieldType);
                                    parser.AddRange(GetParser(item.FieldType));
                                }
                                break;
                            default:
                                throw new NotImplementedException();
                        }
                    }

                    sz = Marshal.SizeOf(type);
                    if (sz != pos)
                    {
                        parser.Add(new Tuple<int, TypeCode>(sz - pos, TypeCode.Empty));
                    }
                }
                else
                {
                    int sz;

                    sz = Marshal.SizeOf(type);
                    parser.Add(new Tuple<int, TypeCode>(sz, TypeCode.Byte));
                }
                parserCache.Add(type, parser);
            }

            return parser;
        }

        private void RunParser(List<Tuple<int, TypeCode>> parser, BinaryReader rd)
        {
            foreach (var item in parser)
            {
                /* Assumption: Types of the same size can be interchanged. */
                switch (item.Item2)
                {
                    case TypeCode.Byte:
                    case TypeCode.SByte:
                        Write(rd.ReadBytes(item.Item1), 0, item.Item1);
                        break;
                    case TypeCode.Int16:
                    case TypeCode.UInt16:
                        for (int i = 0; i < item.Item1; i++)
                            Write(rd.ReadInt16());
                        break;
                    case TypeCode.Int32:
                    case TypeCode.UInt32:
                    case TypeCode.Single:
                        for (int i = 0; i < item.Item1; i++)
                            Write(rd.ReadInt32());
                        break;
                    case TypeCode.Int64:
                    case TypeCode.UInt64:
                    case TypeCode.Double:
                        for (int i = 0; i < item.Item1; i++)
                            Write(rd.ReadInt64());
                        break;
                    case TypeCode.Empty:
                        rd.BaseStream.Seek(item.Item1, SeekOrigin.Current);
                        BaseStream.Seek(item.Item1, SeekOrigin.Current);
                        break;
                    default:
                        throw new NotImplementedException();
                }
            }
        }

        public void Write(object structure)
        {
            List<Tuple<int, TypeCode>> parser;
            Type type;
            byte[] data;

            type = structure.GetType();
            parser = GetParser(type);
            data = new byte[Marshal.SizeOf(type)];

            using (var ms = new MemoryStream(data))
            {
                using (var rd = new BinaryReader(ms))
                {
                    Marshal.StructureToPtr(structure, Marshal.UnsafeAddrOfPinnedArrayElement(data, 0), true);
                    RunParser(parser, rd);
                }
            }
        }

        public void Write(Array structures)
        {
            List<Tuple<int, TypeCode>> parser;
            Type type;
            byte[] data;

            type = structures.GetType().GetElementType();
            parser = GetParser(type);
            data = new byte[Marshal.SizeOf(type)];

            using (var ms = new MemoryStream(data))
            {
                using (var rd = new BinaryReader(ms))
                {
                    foreach (var structure in structures)
                    {
                        ms.Seek(0, SeekOrigin.Begin);
                        Marshal.StructureToPtr(structure, Marshal.UnsafeAddrOfPinnedArrayElement(data, 0), true);
                        RunParser(parser, rd);
                    }
                }
            }
        }

        public void Close()
        {
            BaseStream.Close();
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            if (!disposed)
            {
                if (disposing)
                {
                }

                BaseStream = null;
                buffer = null;

                disposed = true;
            }
        }
    }
}
