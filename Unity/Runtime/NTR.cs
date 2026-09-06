using System;
using System.Runtime.InteropServices;

namespace MayhemDebugger.NetTrace
{
    public static class NTR
    {
        private const string DllName = "NetTraceUnityBridge";
        private const int kMaxValuesPerEvent = 4;

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
        private struct NtrValueFlat
        {
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)] public string Name;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 48)] public string Formatted;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
        private struct NtrEventFlat
        {
            public ulong TimestampMs;
            public uint  SizeBytes;
            public int   Direction;
            public int   ValueCount;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
            public string Name;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = kMaxValuesPerEvent)]
            public NtrValueFlat[] Values;
        }

        [DllImport(DllName, CharSet = CharSet.Ansi)]
        private static extern void ntr_record_event(int direction, string name, uint sizeBytes,
            string[] valueNames, string[] valueFormatted, int valueCount);

        [DllImport(DllName)] private static extern ulong ntr_total_bytes_sent();
        [DllImport(DllName)] private static extern ulong ntr_total_bytes_received();
        [DllImport(DllName)] private static extern ulong ntr_total_events_sent();
        [DllImport(DllName)] private static extern ulong ntr_total_events_received();
        [DllImport(DllName)] private static extern int   ntr_event_count();
        [DllImport(DllName)] private static extern int   ntr_get_event(int indexFromOldest, ref NtrEventFlat outEvent);
        [DllImport(DllName)] private static extern void  ntr_reset_for_testing();

        public enum Direction { Send = 0, Receive = 1 }

        public struct NetworkEvent
        {
            public Direction Direction;
            public string    Name;
            public ulong     TimestampMs;
            public uint      SizeBytes;
            public string[]  ValueNames;
            public string[]  ValueFormatted;
        }

        public struct EventBuilder
        {
            private readonly Direction _direction;
            private readonly string    _name;
            private uint   _sizeBytes;
            private int    _valueCount;
            private readonly string[] _valueNames;
            private readonly string[] _valueFormatted;

            internal EventBuilder(Direction direction, string name)
            {
                _direction      = direction;
                _name           = name;
                _sizeBytes      = 0;
                _valueCount     = 0;
                _valueNames     = new string[kMaxValuesPerEvent];
                _valueFormatted = new string[kMaxValuesPerEvent];
            }

            public EventBuilder Bytes(uint sizeBytes) { _sizeBytes = sizeBytes; return this; }

            public EventBuilder Value(string name, float  v) => AddValue(name, v.ToString("0.00"));
            public EventBuilder Value(string name, double v) => AddValue(name, v.ToString("0.00"));
            public EventBuilder Value(string name, int    v) => AddValue(name, v.ToString());
            public EventBuilder Value(string name, bool   v) => AddValue(name, v ? "true" : "false");
            public EventBuilder Value(string name, string v) => AddValue(name, v);

            private EventBuilder AddValue(string name, string formatted)
            {
                if (_valueCount < kMaxValuesPerEvent)
                {
                    _valueNames[_valueCount]     = name;
                    _valueFormatted[_valueCount] = formatted;
                    _valueCount++;
                }
                return this;
            }

            public void Record()
            {
                ntr_record_event((int)_direction, _name, _sizeBytes,
                    _valueNames, _valueFormatted, _valueCount);
            }
        }

        public static EventBuilder RecordSend(string name)    => new EventBuilder(Direction.Send,    name);
        public static EventBuilder RecordReceive(string name) => new EventBuilder(Direction.Receive, name);

        public static ulong GetTotalBytesSent()      => ntr_total_bytes_sent();
        public static ulong GetTotalBytesReceived()  => ntr_total_bytes_received();
        public static ulong GetTotalEventsSent()     => ntr_total_events_sent();
        public static ulong GetTotalEventsReceived() => ntr_total_events_received();
        public static int   GetEventCount()          => ntr_event_count();

        public static NetworkEvent? GetEvent(int indexFromOldest)
        {
            var flat = new NtrEventFlat { Values = new NtrValueFlat[kMaxValuesPerEvent] };
            if (ntr_get_event(indexFromOldest, ref flat) == 0) return null;

            int count      = Math.Max(0, Math.Min(flat.ValueCount, kMaxValuesPerEvent));
            var names      = new string[count];
            var formatted  = new string[count];
            for (int i = 0; i < count; i++)
            {
                names[i]     = flat.Values[i].Name;
                formatted[i] = flat.Values[i].Formatted;
            }
            return new NetworkEvent
            {
                Direction   = flat.Direction == 0 ? Direction.Send : Direction.Receive,
                Name        = flat.Name,
                TimestampMs = flat.TimestampMs,
                SizeBytes   = flat.SizeBytes,
                ValueNames  = names,
                ValueFormatted = formatted,
            };
        }

        public static void ResetForTesting() => ntr_reset_for_testing();
    }
}
