using System;
using System.Collections.Generic;
using System.IO;

namespace TabParser
{
    class Program
    {
        private const char whiteSpace = (char)32;

        class fret
        {
            public fret(int startIn, int endIn, int locationIn, int strIn)
            {
                start = startIn;
                end = endIn;
                location = locationIn;
                str = strIn;
            }
            public int start;
            public int end;
            public int location;
            public int str;
        }

        static void Main(string[] args)
        {
            string Filename = (args.Length == 1) ? args[0] : Console.ReadLine();
            double currentLocation = 1;
            double[] fretLocations = new double[22];
            for (int i = 0; i < fretLocations.Length; i++)
            {
                fretLocations[i] = currentLocation;
                currentLocation *= (1 - 1 / 17.817);
            }
            StreamReader reader = new StreamReader("rawSongFiles\\"+Filename);
            StreamWriter writer = new StreamWriter("parseOut\\"+Filename+".out");
            string line = "";
            int lineIndex = -2;
            List<List<string>> information = new List<List<string>>();
            while (!reader.EndOfStream)
            {
                line = reader.ReadLine();
                if (lineIndex == -2)
                {
                    line = "{ " + line.ToLower();
                    line = line.Replace(",", " } { ");
                    line = line + " } ";
                    writer.Write(line.Replace("{  }", ""));
                }
                if (lineIndex >= 0)
                {
                    List<string> newList = new List<string>();
                    string[] strings = line.Split(',');
                    for (int i = 1; i < strings.Length; i++)
                    {
                        newList.Add(strings[i]);
                    }
                    information.Add(newList);
                }
                lineIndex++;
            }
            //Tracks current fret being held at every subdivision on each string
            List<List<int>> currentFret = new List<List<int>>();
            //Strings for all extra data, indexed by string and by subdivision
            List<List<string>> specialInstructions = new List<List<string>>();
            //Emphasis of each subdivision
            List<int> emphases = new List<int>();
            //List of all fret instructions in the form of the fret class
            List<fret> fretting = new List<fret>();
            for (int i = 0; i < 6; i++)
            {
                specialInstructions.Add(new List<string>());
                currentFret.Add(new List<int>());
                for (int j = 0; j < information[i].Count; j++)
                {
                    if (information[i][j].Trim() != "")
                    {
                        int nextValue = -1;
                        string[] breakdown = information[i][j].Split(whiteSpace);
                        if (int.TryParse(breakdown[0], out nextValue))
                        {
                            currentFret[i].Add(nextValue);
                        }
                        else if (information[i][j].Contains("x") || information[i][j].Contains("d"))
                        {
                            currentFret[i].Add(-1);
                        }
                        string special = "";
                        if (breakdown.Length > 1)
                        {
                            for (int k = 1; k < breakdown.Length; k++)
                            {
                                special = special + breakdown[k] + whiteSpace;
                            }
                        }
                        specialInstructions[i].Add(special);
                    }
                    else
                    {
                        if (j != 0)
                        {
                            currentFret[i].Add(currentFret[i][j - 1]);
                        }
                        else
                        {
                            currentFret[i].Add(-1);
                        }
                        specialInstructions[i].Add("");
                    }
                }
            }
            for (int j = 0; j < currentFret[0].Count; j++)
            {
                int lowestCurrent = 100;
                for (int i = 0; i < 6; i++)
                {
                    if (currentFret[i][j] < lowestCurrent && currentFret[i][j] != -1 && currentFret[i][j] != 0)
                    {
                        lowestCurrent = currentFret[i][j];
                    }
                }
            }
            for (int j = 0; j < information[8].Count; j++)
            {
                int t = 0;
                if (int.TryParse(information[8][j], out t))
                {
                    emphases.Add(int.Parse(information[8][j]));
                }
                else
                {
                    emphases.Add(0);
                }
            }
            for (int i = 0; i < 6; i++)
            {
                int change = -1;
                int start = 0;
                int end = 0;
                for (int j = 0; j < currentFret[i].Count; j++)
                {
                    if (currentFret[i][j] != change)
                    {
                        end = j;
                        if (start != end)
                        {
                            fretting.Add(new fret(start, end, change, i));
                        }
                        start = j;
                        change = currentFret[i][j];
                    }
                }
                end = currentFret[i].Count;
                if (start != end)
                {
                    fretting.Add(new fret(start, end, change, i));
                }
            }
            foreach (fret e in fretting)
            {
                double pull = 0;
                if (e.location != -1)
                {
                    fret next = new fret(0, 0, 0, -1);
                    foreach (fret f in fretting)
                    {
                        if (f.str == e.str && f.start == e.start)
                        {
                            next = f;
                        }
                    }
                    if (next.str != -1 && specialInstructions[next.str][next.start].Contains("p"))
                    {
                        pull = emphases[next.start];
                        if (!specialInstructions[next.str][next.start].Contains("b"))
                        {
                            double[] bend = new double[e.end - e.start + 1];
                            bend[e.end - e.start] = 0.25;
                            writer.Write(bendString(e.start, e.end - e.start, e.str, 0, bend));
                        }
                    }
                    if (next.str != -1 && specialInstructions[e.str][next.start].Contains("s"))
                    {
                        int diff = next.location - e.location;
                        double step = 1 / (double)Math.Abs(diff);
                        writer.Write(fretString(e.start, e.end - 1 + step, e.location, e.str, pull));
                        for (int i = 1; i < Math.Abs(diff); i++)
                        {
                            writer.Write(fretString(next.start - 1 + i * step, next.start - 1 + (i + 1) * step, e.location + i * Math.Sign(diff), e.str, 0));
                        }
                    }
                    else if (next.location != -1)
                    {
                        writer.Write(fretString(e.start, e.end, e.location, e.str, pull));
                    }
                    else
                    {
                        writer.Write(fretString(e.start, e.end + 0.75, e.location, e.str, pull));
                    }
                }
                else
                {
                    double mute = 0;
                    string[] breakdown = specialInstructions[e.str][e.start].Split(' ');
                    if (( breakdown.Length < 2 ) || !double.TryParse(breakdown[1], out mute))
                    {
                        mute = 24;
                    }
                    int muteEnd = e.start + 1;
                    for (int j = e.start; j < specialInstructions[0].Count; j++)
                    {
                        if (currentFret[e.str][j] == -1)
                        {
                            muteEnd = j + 1;
                        }
                        else
                        {
                            break;
                        }
                    }
                    //Experimental, muting at constant location may cause problems
                    writer.Write(muteString(e.start, muteEnd, e.str, mute, 0.4, 0.1));
                }
            }
            for (int i = 0; i < emphases.Count; i++)
            {
                int firstString = -1;
                if (emphases[i] != 0)
                {
                    int delay = 0;
                    int boost = 0;
                    if (information[6][i].Contains("strum")) 
                    {
                        int strum = 0;
                        string[] breakdown = information[6][i].Split(whiteSpace);
                        for (int j = 0; j < breakdown.Length; j++)
                        {
                            if (breakdown[j] == "strum")
                            {
                                strum = j;
                            }
                        }
                        delay = int.Parse(breakdown[strum + 1]);
                    }
                    else
                    {
                        delay = 500 - emphases[i] * 15;
                    }
                    if (information[7][i] == "down")
                    {
                        int str = 0;
                        while (firstString == -1 && str < 6)
                        {
                            if (information[str][i] != "" && !information[str][i].Contains("x") && !information[str][i].Contains("h") && !information[str][i].Contains("p"))
                            {
                                firstString = str;
                            }
                            str++;
                        }
                        if (firstString != -1)
                        {
                            for (int j = firstString; j < 6; j++)
                            {
                                if (specialInstructions[j][i].Contains("+"))
                                {
                                    boost = 1;
                                }
                                if (information[j][i].Contains("++"))
                                {
                                    boost = 2;
                                }
                                if (information[j][i].Contains("+++"))
                                {
                                    boost = 3;
                                }
                                if (information[j][i] != "" && !information[j][i].Contains("x") && !information[j][i].Contains("h") && !information[j][i].Contains("p"))
                                {
                                    writer.Write(pickString(i, (j - firstString) * delay, emphases[i] + boost, j));
                                }
                            }
                        }
                    }
                    else
                    {

                        int str = 5;
                        while (firstString == -1 && str >= 0)
                        {
                            if (information[str][i] != "" && !information[str][i].Contains("x") && !information[str][i].Contains("h") && !information[str][i].Contains("p"))
                            {
                                firstString = str;
                            }
                            str--;
                        }
                        for (int j = firstString; j >= 0; j--)
                        {
                            if (information[j][i] != "" && !information[j][i].Contains("x") && !information[j][i].Contains("h") && !information[j][i].Contains("p"))
                            {
                                if (specialInstructions[j][i].Contains("+"))
                                {
                                    boost = 1;
                                }
                                if (information[j][i].Contains("++"))
                                {
                                    boost = 2;
                                }
                                if (information[j][i].Contains("+++"))
                                {
                                    boost = 3;
                                }
                                writer.Write(pickString(i, (firstString - j) * delay, emphases[i] + boost, j));;
                            }
                        }
                    }
                }
            }
            foreach (fret e in fretting)
            {
                if (specialInstructions[e.str][e.start].Contains("b"))
                {
                    double current = 0;
                    int duration = e.end - e.start;
                    double[] bend = new double[duration + 1];
                    for (int i = 0; i < duration; i++)
                    {
                        if (specialInstructions[e.str][e.start + i].Contains("b"))
                        {
                            int b = 0;
                            string[] breakdown = specialInstructions[e.str][e.start + i].Split((char)whiteSpace);
                            for (int j = 0; j < breakdown.Length; j++)
                            {
                                if (breakdown[j] == "b")
                                {
                                    b = j;
                                }
                            }
                            current = double.Parse(breakdown[b + 1]);
                            current = Math.Sqrt(Math.Pow(2, current / 6.0) - 1);
                        }
                        bend[i] = current;
                    }
                    writer.Write(bendString(e.start, duration, e.str, 0.6, bend));
                }
            }
            for (int j = 0; j < 6; j++)
            {
                for (int i = 0; i < specialInstructions[j].Count; i++)
                {
                    if (specialInstructions[j][i].Contains("a"))
                    {
                        string[] breakdown = specialInstructions[j][i].Split(whiteSpace);
                        int a = 0;
                        for (int k = 0; k < breakdown.Length; k++)
                        {
                            if (breakdown[k] == "a")
                            {
                                a = k;
                            }
                        }
                        writer.Write(muteString(i, i + 1, j, 32, 0.01, fretLocations[currentFret[j][i]] / (double.Parse(breakdown[a + 1]) + 1)));
                    }
                    if (specialInstructions[j][i].Contains("v"))
                    {
                        int duration = 1;
                        while(i + duration < specialInstructions[j].Count && emphases[i + duration] == 0)
                        {
                            duration++;
                        }
                        string[] breakdown = specialInstructions[j][i].Split(whiteSpace);
                        int v = 0;
                        for (int k = 0; k < breakdown.Length; k++)
                        {
                            if (breakdown[k] == "v")
                            {
                                v = k;
                            }
                        }
                        writer.Write(bendString(i, duration, j, double.Parse(breakdown[v + 1]), new double[duration + 1]));
                    }
                    if (specialInstructions[j][i].Contains("/"))
                    {
                        string[] breakdown = specialInstructions[j][i].Split(whiteSpace);
                        int slash = -1;
                        for (int k = 0; k < breakdown.Length; k++)
                        {
                            if (breakdown[k] == "/")
                            {
                                slash = k;
                            }
                        }
                        int change = -1;
                        int p = 0;
                        for (int k = i + 1; k < information[j].Count; k++)
                        {
                            if (change == -1 && information[j][k] != "")
                            {
                                if (int.TryParse(information[j][k].Split(whiteSpace)[0], out p))
                                {
                                    if (int.Parse(breakdown[slash + 1]) != p)
                                    {
                                        change = k;
                                    }
                                }
                            }
                        } 
                        if (change == -1)
                        {
                            writer.Write(fretString(i + 0.25, information[0].Count - 0.1, int.Parse(breakdown[slash + 1]), j, 0));
                        }
                        else
                        {
                            writer.Write(fretString(i + 0.25, change - 0.1, int.Parse(breakdown[slash + 1]), j, 0));
                        }
                    }
                }
            }
            for (int i = 0; i < currentFret[0].Count; i++)
            {
                if (information[6][i].Contains("mute"))
                {
                    int end = currentFret[0].Count;
                    int mute = 0;
                    string[] breakdown = information[6][i].Split(whiteSpace);
                    for (int j = 0; j < breakdown.Length; j++)
                    {
                        if (breakdown[j] == "mute")
                        {
                            mute = j;
                        }
                    }
                    int k = i;
                    while (end == currentFret[0].Count && k < information[6].Count)
                    {
                        if (information[6][k].Contains("normal"))
                        {
                            end = k;
                        }
                        k++;
                    }
                    for (int j = 0; j < 6; j++)
                    {
                        writer.Write(muteString(i, end, j, double.Parse(breakdown[mute + 1]), 0.1, 0.05));
                    }
                }
            }
            writer.Write("#");
            writer.Close();
        }

        static string fretString(double start, double end, int fret, int str, double pull)
        {
            if (fret == 0)
            {
                return "";
            }
            string output = "{ ";
            output += "fretting : ";
            output += "start = " + start.ToString() + ", ";
            output += "end = " + end.ToString() + ", ";
            output += "fret = " + fret.ToString() + ", ";
            output += "string = " + str.ToString() + ", ";
            output += "pulloff = " + pull.ToString() + ", ";
            output += " } ";
            return output;
        }

        static string muteString(double start, double end, int str, double force, double width, double location)
        {
            string output = "{ ";
            output += "muting : ";
            output += "start = " + start.ToString() + ", ";
            output += "end = " + end.ToString() + ", ";
            output += "force = " + force.ToString() + ", ";
            output += "string = " + str.ToString() + ", ";
            output += "width = " + width.ToString() + ", ";
            output += "location = " + location.ToString() + ", ";
            output += " } ";
            return output;
        }

        static string pickString(double start, double delay, double force, int str)
        {
            string output = "{ ";
            output += "picking : ";
            output += "start = " + start.ToString() + ", ";
            output += "delay = " + delay.ToString() + ", ";
            output += "force = " + force.ToString() + ", ";
            output += "string = " + str.ToString() + ", ";
            output += " } ";
            return output;
        }

        static string bendString(int start, int length, int str, double vibrato, double[] bend)
        {
            string output = "{ ";
            output += "bending : ";
            output += "start = " + start.ToString() + ", ";
            output += "length = " + length.ToString() + ", ";
            output += "string = " + str.ToString() + ", ";
            output += "vibrato = " + vibrato.ToString() + ", ";
            output += "semitones = [ ";
            for (int i = 0; i < bend.Length; i++)
            {
                output += bend[i].ToString() + ", ";
            }
            output += " ] ";
            output += " } ";
            return output;
        }
    }
}
