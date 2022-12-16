package wavingsketch;

import java.util.Map;

public class WavingSketch implements java.io.Serializable {
    private final String NAME;
    private final int SLOT_NUM;
    private final int BUCKET_NUM;
    private final Bucket[] buckets;
    private static final int[] count = {1, -1};

    public WavingSketch(int slotNum, int bucketNum) {
        NAME = "WavingSketch<" + slotNum + ">";
        SLOT_NUM = slotNum;
        BUCKET_NUM = bucketNum;
        buckets = new Bucket[bucketNum];
        for (int i = 0; i < bucketNum; i++) {
            buckets[i] = new Bucket(slotNum);
        }
    }

    public void insert(int item) {
        int bucketPos = Integer.remainderUnsigned(Murmur.hash(item), BUCKET_NUM);
        buckets[bucketPos].insert(item);
    }

    public int query(int item) {
        int bucketPos = Integer.remainderUnsigned(Murmur.hash(item), BUCKET_NUM);
        return buckets[bucketPos].query(item);
    }

    public void check(Map<Integer, Integer> mp, final int HIT) {
        int value, all = 0, hit = 0, size = 0;
        double are = 0, cr, pr;
        for (Map.Entry<Integer, Integer> entry : mp.entrySet()) {
            value = query(entry.getKey());
            if (entry.getValue() > HIT) {
                all++;
                if (value > HIT) {
                    hit += 1;
                    are += Math.abs(entry.getValue() - value) / (double) entry.getValue();
                }
            }
            if (value > HIT)
                size += 1;
        }
        are /= hit;
        cr = hit / (double) all;
        pr = hit / (double) size;

        System.out.printf("%12s:\tARE: %f\tCR: %f\tPR: %f\n", NAME, are, cr, pr);
    }

    class Bucket implements java.io.Serializable {
        private final int[] items;
        private final int[] counters;
        private int incast;

        public Bucket(int slotNum) {
            items = new int[slotNum];
            counters = new int[slotNum];
            incast = 0;
        }

        public void insert(int item) {
            int choice = Murmur.hash(item, 17) & 1;
            int minNum = Integer.MAX_VALUE;
            int minPos = -1;
            for (int i = 0; i < SLOT_NUM; i++) {
                if (counters[i] == 0) {
                    items[i] = item;
                    counters[i] = -1;
                    return;
                } else if (items[i] == item) {
                    if (counters[i] < 0)
                        counters[i]--;
                    else {
                        counters[i]++;
                        incast += count[choice];
                    }
                    return;
                }

                int counterVal = Math.abs(counters[i]);
                if (counterVal < minNum) {
                    minNum = counterVal;
                    minPos = i;
                }
            }

            if (incast * count[choice] >= minNum) {
                if (counters[minPos] < 0) {
                    int minChoice = Murmur.hash(items[minPos], 17) & 1;
                    incast -= count[minChoice] * counters[minPos];
                }
                items[minPos] = item;
                counters[minPos] = minNum + 1;
            }
            incast += count[choice];
        }

        int query(int item) {
            for (int i = 0; i < SLOT_NUM; i++) {
                if (items[i] == item) {
                    return Math.abs(counters[i]);
                }
            }
            return 0;
        }
    }
}
