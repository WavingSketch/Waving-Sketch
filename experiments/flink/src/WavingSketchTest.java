package wavingsketch;

import java.io.DataInputStream;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

public class WavingSketchTest {
    private static ArrayList<Integer> readData(String path) throws FileNotFoundException {
        var items = new ArrayList<Integer>();
        var in = new DataInputStream(new FileInputStream(path));
        while (true) {
            try {
                int temp = in.readInt();
                items.add(temp);
            } catch (IOException e) {
                break;
            }
        }
        return items;
    }

    private static int getTopK(Map<Integer, Integer> mp, int k) {
        var num = new ArrayList<Integer>(mp.size());
        for (Map.Entry<Integer, Integer> entry : mp.entrySet())
            num.add(entry.getValue());
        Collections.sort(num);
        return num.get(mp.size() - k);
    }

    public static void main(String[] args) throws FileNotFoundException {
        System.out.println("Test on ARE, CR and PR");
        System.out.println();

        ArrayList<Integer> items = readData("syn.dat");

        int memInc = 50000, memVar = 6, cmpNum = 4;
        var sketches = new WavingSketch[memVar][cmpNum];

        for (int i = 0; i < memVar; i++) {
            sketches[i][0] = new WavingSketch(4, (i + 1) * memInc / 36);
            sketches[i][1] = new WavingSketch(8, (i + 1) * memInc / 68);
            sketches[i][2] = new WavingSketch(16, (i + 1) * memInc / 132);
            sketches[i][3] = new WavingSketch(32, (i + 1) * memInc / 260);
        }

        var mp = new HashMap<Integer, Integer>();
        for (Integer item : items)
            mp.put(item, mp.get(item) == null ? 1 : mp.get(item) + 1);
        int topK = getTopK(mp, 2000);
        for (int i = 0; i < memVar; i++) {
            System.out.println("Memory size: " + (memInc * (i + 1) / 1000) + "KB");
            System.out.println();
            for (int j = 0; j < cmpNum; j++) {
                for (Integer item : items) {
                    sketches[i][j].insert(item);
                }
            }
            for (int j = 0; j < cmpNum; j++) {
                sketches[i][j].check(mp, topK);
            }
            System.out.println();
        }
    }
}
