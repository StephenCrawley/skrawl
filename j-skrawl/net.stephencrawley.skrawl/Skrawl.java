package net.stephencrawley.skrawl;

import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.io.IOException;

public class Skrawl
{
    public static void main(String[] args) throws IOException
    {
        // run from cmdline
        // TODO read&execute from file
        runPrompt();
    }

    /*
    // read and execute lines from file
    private static void runScript(String[] file)
    {
        // TODO - read from file and call run method
    }
    */

    // read and execute line from the command line
    private static void runPrompt() throws IOException
    {
        // to objects to rea from stdin
        InputStreamReader input = new InputStreamReader(System.in);
        BufferedReader reader = new BufferedReader(input);
        // while (true) is same as for (;;)
        // above for sytax is borrowed from C!
        while (true)
        {
            // print prompt
            System.out.print("skrawl) ");
            // read from stdin
            String line = reader.readLine();
            // if line is null, break from loop
            if (line == null) break;
            // parse the line in our runner
            run(line);
        }
    }

    private static void run(String source)
    {
        // for now - we just print
        System.out.println(source);
    }
}
