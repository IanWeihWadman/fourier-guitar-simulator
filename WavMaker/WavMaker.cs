using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using NAudio;
using System.IO;

namespace WavMaker
{
    class Program
    {
        static void Main(string[] args)
        {
            string songName = (args.Length == 1) ? args[0] : Console.ReadLine();
            string simPath = "simOut\\" + songName + "\\";
            //string[] filenames = new string[] { simPath + "output", simPath + "EFile", simPath + "AFile", simPath + "DFile", simPath + "GFile", simPath + "BFile", simPath + "HighEFile" };
            string[] filenames = new string[] { simPath + "output" };
            foreach (string e in filenames)
            {
                StreamReader file = new StreamReader(e + ".txt");
                List<Int16> values = new List<Int16>();
                while (!file.EndOfStream)
                {
                    string line = file.ReadLine();
                    string[] stringValues = line.Split(',');
                    for (int i = 0; i < stringValues.Length; i++)
                    {
                        double h;
                        if (double.TryParse(stringValues[i], out h))
                        {
                            values.Add((Int16)Math.Round(32768 * double.Parse(stringValues[i])));
                        }
                    }
                }
                Int16[] buffer = values.ToArray();
                FileStream outStream = new FileStream(e + ".wav", FileMode.Create);
                NAudio.Wave.WaveFormat format = new NAudio.Wave.WaveFormat(44100, 16, 1);
                NAudio.Wave.WaveFileWriter writer = new NAudio.Wave.WaveFileWriter(outStream, format);
                writer.WriteSamples(buffer, 0, buffer.Length);
                writer.Close();
            }
            System.IO.File.Copy(filenames[0] + ".wav", "simOut\\" + songName + ".wav", true);
        }
    }
}
