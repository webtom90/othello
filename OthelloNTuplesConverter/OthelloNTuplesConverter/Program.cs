using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace OthelloNTuplesConverter
{
    class Program
    {
        class NTuple
        {
            public int n;
            public int m;
            public List<List<int>> fields = new List<List<int>>();
            public List<double> weights = new List<double>();

            public NTuple(int n, int m)
            {
                this.n = n;
                this.m = m;
            }

            public List<int> getNewFieldsList()
            {
                if (fields.Count >= m)
                    throw new Exception("Zbyt wiele elementów w tablicy pól.");
                var list = new List<int>();
                fields.Add(list);
                return list;
            }

            public int getNumberOfWeights()
            {
                return (int)Math.Pow(3, n);
            }

            public bool check()
            {
                if (fields.Count != m)
                    return false;
                var n = this.n;
                if (fields.FindIndex((l) => { return l.Count != n; }) >= 0)
                    return false;

                if (weights.Count != getNumberOfWeights())
                    return false;

                return true;
            }
        }

        static int convertToBoard(int field)
        {
            int x = field % 8 + 1;
            int y = field / 8 + 1;
            return 10 * x + y;
        }

        static int convertToOth(int field)
        {
            int x = field % 10 - 1;
            int y = field / 10 - 1;
            return 8 * x + y;
        }

        static void convertOth(String inputFilename, String outputFilename)
        {
            var fileTxt = File.ReadAllText(inputFilename);
            var array = fileTxt.Split(new[] { ' ', '\n', '\r', '{', '}' }, StringSplitOptions.RemoveEmptyEntries);

            int index = 0;
            var nTuples = Convert.ToInt32(array[index++]);
            List<NTuple> list = new List<NTuple>();
            for (int i = 0; i < nTuples; i++)
            {
                int n = Convert.ToInt32(array[index++]);
                int m = Convert.ToInt32(array[index++]);
                var tuple = new NTuple(n, m);
                for (int seq = 0; seq < m; seq++)
                {
                    var fields = tuple.getNewFieldsList();
                    for (int fieled = 0; fieled < n; fieled++)
                    {
                        var val = array[index++];
                        fields.Add(convertToBoard(Convert.ToInt32(val)));
                    }
                }
                for (int weight = 0; weight < tuple.getNumberOfWeights(); weight++)
                {
                    var val = array[index++];
                    double value = 0;
                    try
                    {
                        value = Convert.ToDouble(val);
                    }
                    catch
                    {
                        value = Convert.ToDouble(val.Replace(".", ","));
                    }
                    tuple.weights.Add(value);
                }
                if (!tuple.check())
                    throw new Exception("Niepoprawny NTuple!");
                list.Add(tuple);
            }

            using (var output = new BinaryWriter(new FileStream(outputFilename, FileMode.Create)))
            {
                // format pliku
                output.Write((int)0);
                int nF = 0;
                int nW = 0;
                int nT = 0;
                var fieldsMove = new List<int>();
                var weightsMove = new List<int>();
                weightsMove.Add(0);
                fieldsMove.Add(0);
                for (int i = 0; i < list.Count; i++)
                {
                    var t = list[i];
                    for (int j = 0; j < t.m; j++)
                    {
                        fieldsMove.Add(fieldsMove[fieldsMove.Count - 1] + t.n);
                        nF += t.n;
                    }
                    weightsMove.Add(weightsMove[weightsMove.Count - 1] + t.getNumberOfWeights());
                    nW += t.getNumberOfWeights();
                    nT += t.m;
                }
                output.Write(nF);
                output.Write(nW);
                output.Write(nT);
                for (int i = 0; i < list.Count; i++)
                {
                    var t = list[i];
                    for (int j = 0; j < t.m; j++)
                        for (int k = 0; k < t.n; k++)
                            output.Write(t.fields[j][k]);
                }
                for (int i = 0; i < list.Count; i++)
                {
                    var t = list[i];
                    for (int k = 0; k < t.getNumberOfWeights(); k++)
                        output.Write(t.weights[k]);
                }
                index = 0;
                for (int i = 0; i < list.Count; i++)
                {
                    var t = list[i];
                    for (int j = 0; j < t.m; j++)
                    {
                        output.Write(t.n);
                        output.Write(fieldsMove[index]);
                        output.Write(weightsMove[i]);
                        index++;
                    }
                }
            }
        }

        static void convertWPC(String inputFilename, String outputFilename)
        {
            var fileTxt = File.ReadAllText(inputFilename);
            var array = fileTxt.Split(new[] { "\n", "\r\n" }, StringSplitOptions.RemoveEmptyEntries);
            if (array.Length != 64)
            {
                Console.WriteLine("File {0} has {1} lines instead of {2}", inputFilename, array.Length, 64);
                return;
            }

            using (var output = new BinaryWriter(new FileStream(outputFilename, FileMode.Create)))
            {
                // format pliku
                output.Write((int)1);

                for (int i = 0; i < 64; i++)
                {
                    double value = 0;
                    try
                    {
                        value = Convert.ToDouble(array[i].Replace(".", ","));
                    }
                    catch
                    {
                        value = Convert.ToDouble(array[i].Replace(",", "."));
                    }
                    output.Write(value);
                }
            }
        }

        static void convertBoard(String inputFilename, String outputFilename)
        {
            var list = new List<List<int>>();

            using (var input = new StreamReader(inputFilename))
            {
                int lineCounter = 0;
                while (input.Peek() > -1)
                {
                    var line = input.ReadLine();
                    if (String.IsNullOrEmpty(line.Trim()))
                        continue;
                    var array = line.Split(new[] { "[", "]", ",", " " }, StringSplitOptions.RemoveEmptyEntries);
                    if (array.Length != 64)
                    {
                        Console.WriteLine("File {0} has {1} entries instead of {2} in {3} line", inputFilename, array.Length, 64, lineCounter);
                        return;
                    }

                    var tmpList = new List<int>();
                    for (int i = 0; i < 64; i++)
                    {
                        double value = 0;
                        try
                        {
                            value = Convert.ToDouble(array[i].Replace(".", ","));
                        }
                        catch
                        {
                            value = Convert.ToDouble(array[i].Replace(",", "."));
                        }

                        int valueInt = Convert.ToInt32(value);
                        if (valueInt != value)
                        {
                            Console.WriteLine("Precision lost in file {0} line {1} field: {2} vs {3}", inputFilename, lineCounter, value, valueInt);
                            return;
                        }
                        valueInt = -valueInt;
                        tmpList.Add(valueInt);
                    }
                    list.Add(tmpList);
                }
            }

            using (var output = new BinaryWriter(new FileStream(outputFilename, FileMode.Create)))
            {
                // format pliku
                output.Write((int)2);

                output.Write(list.Count);

                foreach (var array in list)
                {
                    foreach (var value in array)
                        output.Write(value);
                }
            }
        }

        static void binToTxtOth(BinaryReader reader, string outputFile)
        {
            int nFields = reader.ReadInt32();
            int nWeights = reader.ReadInt32();
            int nTuples = reader.ReadInt32();
            var fields = new List<int>();
            var weights = new List<double>();
            var tuples = new List<Tuple<int, int, int>>();
            for (int i = 0; i < nFields; i++)
                fields.Add(reader.ReadInt32());
            for (int i = 0; i < nWeights; i++)
                weights.Add(reader.ReadDouble());
            for (int i = 0; i < nTuples; i++)
                tuples.Add(Tuple.Create(reader.ReadInt32(), reader.ReadInt32(), reader.ReadInt32()));

            var t = tuples.GroupBy(t1 => t1.Item3).ToList();

            using (var writer = new StreamWriter(outputFile, false))
            {
                writer.WriteLine("{{ {0}", t.Count);

                for (int i = 0; i < t.Count; i++)
                {
                    var currentTuples = t[i].ToList();
                    int nF = currentTuples[0].Item1;
                    writer.WriteLine("  {{ {0} {1}", nF, currentTuples.Count);
                    for (int j = 0; j < currentTuples.Count; j++)
                    {
                        writer.Write("    { ");

                        for (int k = 0; k < nF; k++)
                        {
                            writer.Write("{0,2} ", convertToOth(fields[currentTuples[j].Item2 + k]));
                        }

                        writer.WriteLine("}");
                    }
                    writer.WriteLine();

                    int nW = 1;
                    for (int j = 0; j < nF; j++)
                        nW *= 3;

                    writer.Write("    { ");

                    for (int j = 0; j < nW; j++)
                    {
                        writer.Write("{0} ", weights[currentTuples[0].Item3 + j].ToString().Replace(',', '.'));
                    }
                    writer.WriteLine("}");
                    writer.WriteLine("  }");
                }

                writer.WriteLine("}");
            }
        }

        static void binToTxtWPC(BinaryReader reader, string outputFile)
        {
            using (var writer = new StreamWriter(outputFile, false))
            {
                for (int i = 0; i < 64; i++)
                {
                    writer.WriteLine(reader.ReadDouble().ToString().Replace(',', '.'));
                }
            }
        }

        static void binToTxtBoard(BinaryReader reader, string outputFile)
        {
            var boards = new List<List<int>>();

            int n = reader.ReadInt32();

            while (reader.PeekChar() >= 0)
            {
                var list = new List<int>();
                for (int i = 0; i < 64; i++)
                {
                    if (reader.PeekChar() < 0)
                    {
                        Console.WriteLine("File corrupted in #{0} board ({1} position, {2})", boards.Count);
                        return;
                    }

                    list.Add(reader.ReadInt32());
                }
                boards.Add(list);
            }

            if (boards.Count != n)
            {
                Console.WriteLine("Readed {0} boards instead of {1}.", boards.Count, n);
            }

            using (var writer = new StreamWriter(outputFile, false))
            {
                foreach (var board in boards)
                {
                    writer.Write("[");
                    for (int i = 0; i < board.Count; i++)
                    {
                        if (i > 0)
                            writer.Write(", ");
                        writer.Write("{0,4}", Convert.ToDouble(board[i]).ToString("0.0").Replace(',', '.'));
                    }
                    writer.WriteLine("]");
                }
            }
        }

        static void binToTxt(string inputFile, string outputFile)
        {
            using (var reader = new BinaryReader(new FileStream(inputFile, FileMode.Open)))
            {
                int type = reader.ReadInt32();
                switch (type)
                {
                    case 0:
                        binToTxtOth(reader, outputFile);
                        break;
                    case 1:
                        binToTxtWPC(reader, outputFile);
                        break;
                    case 2:
                        binToTxtBoard(reader, outputFile);
                        break;
                    default:
                        Console.WriteLine("Nieznany typ: {0}", type);
                        break;
                }
            }
        }

        static void info(string inputFile)
        {
            using (var reader = new BinaryReader(new FileStream(inputFile, FileMode.Open)))
            {
                int type = reader.ReadInt32();
                switch (type)
                {
                    case 0:
                        {
                            Console.WriteLine("NTuples");
                            Console.WriteLine("nFields: {0}", reader.ReadInt32());
                            Console.WriteLine("nWeights: {0}", reader.ReadInt32());
                            Console.WriteLine("nTuples: {0}", reader.ReadInt32());
                        }
                        break;
                    case 1:
                        Console.WriteLine("WPC");
                        break;
                    case 2:
                        Console.WriteLine("Board");
                        break;
                    default:
                        Console.WriteLine("Nieznany typ: {0}", type);
                        break;
                }
            }
        }

        static void Main(string[] args)
        {
            if (args.Length == 0)
            {
                Console.WriteLine("Pierwszy parametr: totxt|tobin");
                return;
            }
            if ("tobin".Equals(args[0].ToLower()))
            {
                if (args.Length != 4)
                {
                    Console.WriteLine("Parametry: (tobin, oth|wpc|pos, input, output)");
                    return;
                }
                if ("oth".Equals(args[1].ToLower()))
                {
                    convertOth(args[2], args[3]);
                }
                else if ("wpc".Equals(args[1].ToLower()))
                {
                    convertWPC(args[2], args[3]);
                }
                else if ("pos".Equals(args[1].ToLower()))
                {
                    convertBoard(args[2], args[3]);
                }
                else
                {
                    Console.WriteLine("Nierozpoznany parametr: {0}", args[1]);
                }
            }
            else if ("totxt".Equals(args[0].ToLower()))
            {
                if (args.Length != 3)
                {
                    Console.WriteLine("Parametry: (totxt, input, output)");
                    return;
                }
                binToTxt(args[1], args[2]);
            }
            else if ("info".Equals(args[0].ToLower()))
            {
                if (args.Length != 2)
                {
                    Console.WriteLine("Parametry: (totxt, input)");
                    return;
                }
                info(args[1]);
            }
            else
            {
                Console.WriteLine("Nierozpoznany parametr: {0}", args[0]);
                return;
            }
        }
    }
}
