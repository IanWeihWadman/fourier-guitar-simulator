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
            public fret(int startIn, int endIn, int locationIn, int strIn, string specialIn)
            {
                start = startIn;
                end = endIn;
                location = locationIn;
                str = strIn;
                special = specialIn;
            }
            public int start;
            public int end;
            public int location;
            public int str;
            public string special;
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
            string songName = Path.GetFileNameWithoutExtension(Filename);
            StreamWriter writer = new StreamWriter("parseOut\\"+songName+".out");
            string line = "";
            int lineIndex = -2;
            //This vector of vectors contains the raw values read from the file as strings
            List<List<string>> information = new List<List<string>>();
            //This section reads the input file
            while (!reader.EndOfStream)
            {
                line = reader.ReadLine();
                //First line is read differently since it contains the header, second line is skipped entirely
                if (lineIndex == -2)
                {
                    line = "{ " + line.ToLower();
                    line = line.Replace(",", " } { ");
                    line = line + " } ";
                    writer.Write(line.Replace("{  }", ""));
                }
                //Remaining lines are read normally into the information vector
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
            int songLength = information[0].Count;
            //Strings for all extra data, indexed by string and by subdivision
            List<int> emphases = new List<int>();
            //List of all fret instructions in the form of the fret class
            List<fret> fretting = new List<fret>();
            for (int i = 0; i < 6; i++)
            {
                //This loop converts the raw instructions into fret objects
                int change = -1;
                string special = "";
                int start = 0;
                for (int j = 0; j < songLength + 1; j++)
                {
                    if (j == songLength) 
                    {
                        //Writes the final fret instruction when the end of the tab is reached
                        fretting.Add(new fret(start, j, change, i, special));
                    }
                    else if (information[i][j].Trim() != "")
                    {
                        //When a new fret instruction is found, the previous fret instruction is recorded before all the values are replaced
                        fretting.Add(new fret(start, j, change, i, special));
                        start = j;
                        int nextValue = 0;
                        string[] breakdown = information[i][j].Split(whiteSpace);
                        //Tries to parse the first value in the cell into a fret number
                        if (int.TryParse(breakdown[0], out nextValue))
                        {
                            change = nextValue;
                        }
                        //The current fret is -1 if the instruction is x or d
                        else if (information[i][j].Contains("x") || information[i][j].Contains("d"))
                        {
                            nextValue = -1;
                            change = -1;
                        }
                        special = "";
                        //If more than just the fret number is included, the rest is copied into the special instructions of the fret instruction
                        if (breakdown.Length > 1)
                        {
                            for (int k = 1; k < breakdown.Length; k++)
                            {
                                special = special + breakdown[k] + whiteSpace;
                            }
                        }
                    }
                }
            }
            for (int j = 0; j < songLength; j++)
            {
                //This loop reads the emphasis line and convert the strings to ints
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
            foreach (fret e in fretting)
            {
                //This loops over all the fret instructions we just generated and writes them to the output file, along with the results of their special instructions
                double pull = 0;
                //We find the next fret instruction on the same string, if none exists this blank fret instruction is not replaced
                fret next = new fret(int.MaxValue, int.MaxValue, 0, -1, "");
                foreach (fret f in fretting)
                {
                    if (f.str == e.str && f.start >= e.end && f.start < next.start)
                    {
                        next = f;
                    }
                }
                if (e.location != -1)
                {
                    //This deals with the instructions that are applied based on the special instructions of the next fret on the same string
                    if (next.str != -1 && next.special.Contains("p"))
                    {
                        pull = emphases[next.start];
                        if (!e.special.Contains("b"))
                        {
                            List<double> bend = new List<double> { };
                            for (int i = 0; i < e.end - e.start; i++)
                            {
                                bend.Add(0);
                            }
                            bend.Add(0.3);
                            writer.Write(bendString(e.start, e.end - e.start, e.str, 0, 0, bend));
                        }
                    }
                    if (next.str != -1 && next.special.Contains("s") && next.location - e.location != 0)
                    {
                        int diff = next.location - e.location;
                        double step = 1 / (double)Math.Abs(diff);
                        writer.Write(fretString(e.start, e.end - 1, e.location, e.str, pull));
                        for (int i = 0; i < Math.Abs(diff) - 1; i++)
                        {
                            writer.Write(fretString(next.start - 1 + i * step, next.start - 1 + (i + 1) * step, e.location + (i + 1) * Math.Sign(diff), e.str, 0));
                        }
                    }
                    else if (next.location != -1)
                    {
                        //Writing the actual instruction to the file
                        writer.Write(fretString(e.start, e.end, e.location, e.str, pull));
                    }
                    else
                    {
                        //If the next instruction is a mute, the fret is held slightly longer to avoid artifacts
                        writer.Write(fretString(e.start, e.end + 0.25, e.location, e.str, pull));
                    }
                }
                else
                {
                    //This deals with the mute instructions x and d
                    double mute = 0;
                    string[] breakdown = e.special.Split(' ');
                    //If no mute intensity is specified, the default value 24 is used
                    if (( breakdown.Length < 2 ) || !double.TryParse(breakdown[1], out mute))
                    {
                        mute = 24;
                    }
                    //The string is muted until a non-mute fret instruction is encountered
                    int muteEnd = songLength;
                    if (next.str != -1)
                    {
                        muteEnd = next.start;
                    }
                    writer.Write(muteString(e.start, muteEnd, e.str, mute, 0.4, 0.1));
                }
            }
            for (int i = 0; i < emphases.Count; i++)
            {
                //This loop creates the pick instructions based on the emphasis line
                int firstString = -1;
                if (emphases[i] != 0)
                {
                    int delay = 0;
                    int boost = 0;
                    //Checks for a strum instruction on the phrasing line
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
                    //Otherwise delay defaults to this value
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
                            //Check for single-string emphasis boosts
                            for (int j = firstString; j < 6; j++)
                            {
                                if (information[j][i].Contains("+"))
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
                                    //Each picking instruction is delayed proportional to the variable delay
                                    writer.Write(pickString(i, (j - firstString) * delay, emphases[i] + boost, j));
                                }
                            }
                        }
                    }
                    else
                    {
                        //Same as above, but reverses string order for upward strumming
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
                                if (information[j][i].Contains("+"))
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
                //This deals with bending
                if (e.special.Contains("b"))
                {
                    double current = 0;
                    int duration = 0;
                    List<double> bend = new List<double> { };
                    while (true)
                    {
                        //Looks for later fret instructions on the same string and same fret
                        fret next = checkForFret(e.str, e.start + duration, fretting);
                        //If a fret instruction is found with no bend instruction, the bend is over
                        if (next.special.Contains("b"))
                        {
                            int b = 0;
                            string[] breakdown = next.special.Split((char)whiteSpace);
                            for (int j = 0; j < breakdown.Length; j++)
                            {
                                if (breakdown[j] == "b")
                                {
                                    b = j;
                                }
                            }
                            //Remove the bend instruction so that duplicate bends are not created
                            next.special = next.special.Replace("b", "");
                            current = double.Parse(breakdown[b + 1]);
                            current = Math.Sqrt(Math.Pow(2, current / 6.0) - 1);
                        }
                        else if (next.str != -1)
                        {
                            break;
                        }
                        bend.Add(current);
                        duration++;
                    }
                    bend.Add(0);
                    writer.Write(bendString(e.start, duration, e.str, 0, 0, bend));
                }
                if (e.special.Contains("a"))
                {
                    //This part deals with pinch harmonics
                    string[] breakdown = e.special.Split(whiteSpace);
                    int a = 0;
                    for (int k = 0; k < breakdown.Length; k++)
                    {
                        if (breakdown[k] == "a")
                        {
                            a = k;
                        }
                    }
                    //Writes a mute at the correct location to produce a harmonic
                    writer.Write(muteString(e.start + 0.1, e.start + 0.4, e.str, 48, 0.01, fretLocations[e.location] / (double.Parse(breakdown[a + 1]) + 1)));
                }
                if (e.special.Contains("v"))
                {
                    //This part deals with vibrato
                    int duration = 1;
                    //Vibrato continues until a non-zero emphasis is encountered or the song ends
                    while (e.start + duration < songLength && emphases[e.start + duration] == 0)
                    {
                        duration++;
                    }
                    string[] breakdown = e.special.Split(whiteSpace);
                    int v = 0;
                    for (int k = 0; k < breakdown.Length; k++)
                    {
                        if (breakdown[k] == "v")
                        {
                            v = k;
                        }
                    }
                    //If no values are parseable, defaults to 1 and 600
                    double vibrato = 1;
                    double vibSpeed = 600;
                    if (double.TryParse(breakdown[v + 1], out vibrato))
                    {
                        if (!double.TryParse(breakdown[v + 2], out vibSpeed))
                        {
                            vibSpeed = 600;
                        }
                    }
                    else
                    {
                        vibrato = 1;
                    }
                    List<double> bend = new List<double> { };
                    for (int i = 0; i < duration + 1; i++)
                    {
                        bend.Add(0);
                    }
                    writer.Write(bendString(e.start, duration, e.str, vibrato, vibSpeed, bend));
                }
                if (e.special.Contains("/"))
                {
                    //This part deals with grace notes
                    string[] breakdown = e.special.Split(whiteSpace);
                    int slash = -1;
                    for (int k = 0; k < breakdown.Length; k++)
                    {
                        if (breakdown[k] == "/")
                        {
                            slash = k;
                        }
                    }
                    writer.Write(fretString(e.start + 0.25, e.end - 0.1, int.Parse(breakdown[slash + 1]), e.str, 0));
                }
            }
            for (int i = 0; i < songLength; i++)
            {
                //This deals with palm muting instructions in the phrasing line
                if (information[6][i].Contains("mute"))
                {
                    int end = songLength;
                    int mute = 0;
                    string[] breakdown = information[6][i].Split(whiteSpace);
                    for (int j = 0; j < breakdown.Length; j++)
                    {
                        if (breakdown[j] == "mute")
                        {
                            mute = j;
                        }
                    }
                    int k = i + 1;
                    while (end == songLength && k < information[6].Count)
                    {
                        if (information[6][k].Contains("normal") || information[6][k].Contains("mute"))
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

        static fret checkForFret(int str, int time, List<fret> allFrets)
        {
            foreach (fret e in allFrets)
            {
                if (e.str == str && e.start == time)
                {
                    return e;
                }
            }
            return new fret(0, 0, -1, -1, "");
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

        static string bendString(int start, int length, int str, double vibrato, double vibSpeed, List<double> bend)
        {
            string output = "{ ";
            output += "bending : ";
            output += "start = " + start.ToString() + ", ";
            output += "length = " + length.ToString() + ", ";
            output += "string = " + str.ToString() + ", ";
            output += "vibrato = " + vibrato.ToString() + ", ";
            output += "vibspeed = " + vibSpeed.ToString() + ", ";
            output += "semitones = [ ";
            for (int i = 0; i < bend.Count; i++)
            {
                output += bend[i].ToString() + ", ";
            }
            output += " ] ";
            output += " } ";
            return output;
        }
    }
}
