package wavingsketch;

public class Murmur {
    private static int rotl32(int x, byte r) {
        return (x << r) | (x >>> (32 - r));
    }

    private static int fmix32(int h) {
        h ^= h >>> 16;
        h *= 0x85ebca6b;
        h ^= h >>> 13;
        h *= 0xc2b2ae35;
        h ^= h >>> 16;
        return h;
    }

    public static int hash(int key, int seed) {
        int h1 = seed;
        int c1 = 0xcc9e2d51;
        int c2 = 0x1b873593;
        int k1 = key;
        k1 *= c1;
        k1 = rotl32(k1, (byte) 15);
        k1 *= c2;
        h1 ^= k1;
        h1 = rotl32(h1, (byte) 13);
        h1 = h1 * 5 + 0xe6546b64;
        h1 ^= 4;
        h1 = fmix32(h1);
        return h1;
    }

    public static int hash(int key) {
        return hash(key, 0);
    }
}
