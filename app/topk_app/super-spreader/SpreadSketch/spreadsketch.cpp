#include "spreadsketch.hpp"

#ifdef HH
DetectorSS::DetectorSS(int depth, int width, int lgn, int b, int c, int memory, int len, unsigned mask) {
    ss_.depth = depth;
    ss_.width = width;
    ss_.lgn = lgn;
    ss_.tdepth = depth > 4 ? 4 : depth;

    //init bitmap
    ss_.b = b;
    ss_.c = c;
    ss_.lastb=(memory>0?memory-(c-1)*b:b*2);
    if (ss_.lastb < b*2) {
        std::cout << "Not enough memory for last component: b=" << b << "  c=" << c
            << "   mem=" << memory << std::endl;
        exit(-1);
    }
    if (log(b)+log(2)*c>=32*log(2)) {
        std::cout << "Multiresolition bitmap too fine for 32 bit hash: b=" << b << "  c="
            << c << std::endl;
        exit(-1);
    }
    if(log(b)+log(2)*c>27*log(2)){
        std::cout << "Unsafe DiscounterMRB for a 32 bit hash: b="<< b << "  c="
            << c << std::endl;
        exit(-1);
    }

    ss_.setmax = (int)(b*bitsetratio+0.5);
    ss_.offsets = new int[c+1]();
    for(int i=0;i<c;i++){ss_.offsets[i]=i*b;}
    ss_.offsets[c] = ss_.offsets[c-1]+ss_.lastb;
    ss_.counts = new unsigned char*[ss_.depth]();
    for (int i = 0; i < ss_.depth; i++) {
        int size = (ss_.offsets[c] + 7) >> 3;
        ss_.counts[i] = new unsigned char[size*ss_.width]();
    }

    //init SSketch
    ss_.skey = new key_tp*[depth];
    ss_.level = new int*[depth];
    for (int i = 0; i < depth; i++) {
        ss_.skey[i] = new key_tp[width]();
        ss_.level[i] = new int[width]();
    }
    ss_.hash = new unsigned long[depth];
    char name[] = "DetectorSS";
    unsigned long seed = AwareHash((unsigned char*)name, strlen(name), 13091204281, 228204732751, 6620830889);
    for (int i = 0; i < depth; i++) {
        ss_.hash[i] = GenHashSeed(seed++);
    }
    ss_.len = len;
    ss_.mask = mask;
    //init HH
    ss_.key = new unsigned long[ss_.len]();
    ss_.indicator = new int[ss_.len]();
}

#else

DetectorSS::DetectorSS(int depth, int width, int lgn, int b, int c, int memory) {
    ss_.depth = depth;
    ss_.width = width;
    ss_.lgn = lgn;
    ss_.tdepth = depth > 4 ? 4 : depth;
    //init bitmap
    ss_.b = b;
    ss_.c = c;
    ss_.lastb=(memory>0?memory-(c-1)*b:b*2);
    if (ss_.lastb < b*2) {
        std::cout << "Not enough memory for last component: b=" << b << "  c=" << c
            << "   mem=" << memory << std::endl;
        exit(-1);
    }
    if (log(b)+log(2)*c>=32*log(2)) {
        std::cout << "Multiresolition bitmap too fine for 32 bit hash: b=" << b << "  c="
            << c << std::endl;
        exit(-1);
    }
    if(log(b)+log(2)*c>27*log(2)){
        std::cout << "Unsafe DiscounterMRB for a 32 bit hash: b="<< b << "  c="
            << c << std::endl;
        exit(-1);
    }

    ss_.setmax = (int)(b*bitsetratio+0.5);
    ss_.offsets = new int[c+1]();
    for(int i=0;i<c;i++){ss_.offsets[i]=i*b;}
    ss_.offsets[c] = ss_.offsets[c-1]+ss_.lastb;
    ss_.counts = new unsigned char*[ss_.depth]();
    for (int i = 0; i < ss_.depth; i++) {
        int size = (ss_.offsets[c] + 7) >> 3;
        ss_.counts[i] = new unsigned char[size*ss_.width]();
    }

    //init SSketch
    ss_.skey = new key_tp*[depth];
    ss_.level = new int*[depth];
    for (int i = 0; i < depth; i++) {
        ss_.skey[i] = new key_tp[width]();
        ss_.level[i] = new int[width]();
    }
    ss_.hash = new unsigned long[depth];
    char name[] = "DetectorSS";
    unsigned long seed = AwareHash((unsigned char*)name, strlen(name), 13091204281, 228204732751, 6620830889);
    for (int i = 0; i < depth; i++) {
        ss_.hash[i] = GenHashSeed(seed++);
    }
}
#endif

DetectorSS::~DetectorSS() {
    delete [] ss_.hash;
    for (int i = 0; i < ss_.depth; i++) {
        delete [] ss_.skey[i];
        delete [] ss_.level[i];
        delete [] ss_.counts[i];
    }
    delete [] ss_.offsets;
    delete [] ss_.counts;
    delete [] ss_.skey;
    delete [] ss_.level;
#ifdef HH
    delete [] ss_.key;
    delete [] ss_.indicator;
#endif
}

//right most zero
int DetectorSS::loghash(unsigned long p) {
    int ret = 0;
    while ((p&0x00000001) == 1) {
        p >>= 1;
        ret++;
    }
    return ret;
}

void DetectorSS::Setbit(int n, int bucket,  unsigned char* bmp) {
    n += bucket*ss_.offsets[ss_.c];
    bmp[(n)>>3]|=(0x80>>((n)&0x07));
}


// For width <= 4
void DetectorSS::Update(key_tp src, key_tp dst, val_tp weight) {

    unsigned long edge = src;
    edge = (edge << 32) | dst;
    int tmplevel = 0;
    //Update sketch
    unsigned long p = MurmurHash64A((unsigned char*)(&edge), ss_.lgn/8*2, ss_.hash[0]);
#ifdef HH
    //check if HH
    int index = p & ss_.mask;
    if (ss_.key[index] == edge) {
        ss_.indicator[index]++;
        return;
    } else {
        ss_.indicator[index]--;
        if (ss_.indicator[index] < 0) {
            ss_.key[index] = edge;
            ss_.indicator[index] = 1;
        }
    }
#endif
    tmplevel = loghash(p);
    unsigned bucket = 0;
    uint32_t key[3] = {src, src, src};
    uint32_t hashval[4] = {0};
    MurmurHash3_x64_128 ( (unsigned char*)key, 12, ss_.hash[0], (unsigned char*)hashval);
    int pos = 0;
    if(tmplevel < ss_.c-1){
        pos = (((uint32_t)p * ((uint64_t)ss_.b)) >> 32);
        pos += ss_.offsets[tmplevel];
    } else {
        pos = (((uint32_t)p * (uint64_t)ss_.lastb) >> 32);
        pos += ss_.offsets[ss_.c-1];
    }

    for (int i = 0; i < ss_.tdepth; i++) {
        bucket = (hashval[i] * (unsigned long)ss_.width) >> 32;
        if (ss_.level[i][bucket] < tmplevel) {
            ss_.level[i][bucket] = tmplevel;
            ss_.skey[i][bucket] = src;
        }
        //Update distinct counter
        Setbit(pos, bucket, ss_.counts[i]);
    }

    for (int i = 4; i < ss_.depth; i++) {
        bucket = MurmurHash64A((unsigned char*)(&src), ss_.lgn/8, ss_.hash[i]);
        bucket = (bucket * (unsigned long)ss_.width) >> 32;
        if (ss_.level[i][bucket] < tmplevel) {
            ss_.level[i][bucket] = tmplevel;
            ss_.skey[i][bucket] = src;
        }
        Setbit(pos, bucket, ss_.counts[i]);
    }


}

int DetectorSS::Estimate(int bucket, unsigned char* bmp){
  int offs = bucket * ss_.offsets[ss_.c];
  int z, tmpoff=0, base;
  double m;
  int factor=1;
  for(base=0; base<ss_.c-1; base++){
    tmpoff = offs + ss_.offsets[base];
    z=countzerobits(bmp,tmpoff,tmpoff+ss_.b);
    //TL: if the bitmap is not full
    if(ss_.b-z <= ss_.setmax){
      break;
    } else {
      factor=factor*2;
    }
  }
  m=0;

  int pos = base-1;
  for(int i=base; i<ss_.c-1; i++){
    tmpoff = offs + ss_.offsets[i];
    z=countzerobits(bmp, tmpoff, tmpoff+ss_.b);
    m+=ss_.b*(log(ss_.b)-log(z));
    if (z == 0 || ss_.b-z > ss_.setmax) {
      pos = i;
      m = 0;
    }
  }
  factor=factor*(1 << (pos-base+1));
  
  tmpoff = offs + ss_.offsets[ss_.c-1];
  z=countzerobits(bmp, tmpoff, tmpoff+ss_.lastb);
  if(z==0){
    m+=ss_.lastb*(log(ss_.lastb));
  } else {
    m+=ss_.lastb*(log(ss_.lastb)-log(z));
  }
  return (int)(factor*m+0.5);
}

int DetectorSS::PointQuery(uint32_t key) {
    uint32_t hkey[3] = {key, key, key};
    uint32_t hashval[4] = {0};
    MurmurHash3_x64_128 ( (unsigned char*)hkey, 12, ss_.hash[0], (unsigned char*)hashval);
    int ret=0;
    int degree=0;
    for (int i=0; i<ss_.tdepth; i++) {
        unsigned bucket = (uint32_t )hashval[i] * (unsigned long)ss_.width >> 32;
        if (i==0) {
            ret = Estimate(bucket, ss_.counts[i]);
        }
        degree = Estimate(bucket, ss_.counts[i]);
        ret = ret > degree ? degree : ret;
    }
    for (int i=4; i<ss_.depth; i++) {
        unsigned bucket = MurmurHash64A((unsigned char*)(&key), ss_.lgn/8, ss_.hash[i]);
        bucket = (bucket * (unsigned long)ss_.width) >> 32;
        degree = Estimate(bucket, ss_.counts[i]);
        ret = ret > degree ? degree : ret;
    }
    return ret;
}

//copy len bits from src that starts at start to dst,
void DetectorSS::Copybmp(unsigned char* dst, unsigned char* src, int start, int len) {

    int curlen = 0;
    while (curlen < len) {
        unsigned char temp = 0;
        int n = start + curlen;
        int index = n >> 3;
        int pos = n&0x07;
        int pos2 = curlen&0x07;
        if ((8-pos2) > (8-pos)) {
            temp |= src[index] & (0xff >> pos);
            temp <<= (pos-pos2);
            dst[curlen>>3] |= temp;
            curlen += 8-pos;
        } else {
            temp |= src[index] & (0xff >> pos);
            temp >>= (pos2-pos);
            dst[curlen>>3] |= temp;
            curlen += 8-pos2;
        }
    }
}

//Intersec two bitmaps
void DetectorSS::Intersec(unsigned char* dst, unsigned char* src, int start, int len) {

    int curlen = 0;
    while (curlen < len) {
        unsigned char temp = 255;
        int n = start + curlen;
        int index = n >> 3;
        int pos = n&0x07;
        int pos2 = curlen&0x07;
        if ((8-pos2) > (8-pos)) {
            temp &= src[index] & (0xff >> pos);
            temp <<= (pos-pos2);
            temp |= ((1 << (pos-pos2)) -1);
            dst[curlen>>3] &= temp;
            curlen += 8-pos;
        } else {
            temp &= src[index] & (0xff >> pos);
            temp >>= (pos2-pos);
            temp |= 256 - (1 << (8-pos2));
            dst[curlen>>3] &= temp;
            curlen += 8-pos2;
        }
    }
}

//Merge then query
int DetectorSS::PointQueryMerge(uint32_t key) {
    int ret=0;
    unsigned char* mcount = new unsigned char[(ss_.offsets[ss_.c]+7)>>3]();
    uint32_t hkey[3] = {key, key, key};
    uint32_t hashval[4] = {0};
    MurmurHash3_x64_128 ( (unsigned char*)hkey, 12, ss_.hash[0], (unsigned char*)hashval);
    for (int i = 0; i < ss_.tdepth; i++) {
        unsigned bucket = (uint32_t )hashval[i] * (unsigned long)ss_.width >> 32;
        if (i == 0) {
            Copybmp(mcount, ss_.counts[i], bucket*ss_.offsets[ss_.c], ss_.offsets[ss_.c]);
        } else
            Intersec(mcount, ss_.counts[i], bucket*ss_.offsets[ss_.c], ss_.offsets[ss_.c]);
    }
    for (int i = 4; i < ss_.depth; i++) {
        unsigned bucket = MurmurHash64A((unsigned char*)(&key), ss_.lgn/8, ss_.hash[i]);
        bucket = (bucket * (unsigned long)ss_.width) >> 32;
        Intersec(mcount, ss_.counts[i], bucket*ss_.offsets[ss_.c], ss_.offsets[ss_.c]);
    }
    ret = Estimate(0, mcount);
    delete [] mcount;
    return ret;
}

void DetectorSS::Query(val_tp thresh, std::vector<std::pair<key_tp, val_tp> > &results) {

    std::unordered_set<key_tp> reset;
    for (int i = 0; i < ss_.depth; i++) {
        for (int j = 0; j < ss_.width; j++) {
            val_tp est = Estimate(j, ss_.counts[i]);
            if ( est >= thresh) {
                reset.insert(ss_.skey[i][j]);
            }
        }
    }

    for (auto it = reset.begin(); it != reset.end(); it++) {
        val_tp degree = PointQueryMerge(*it);
        if ( degree >= thresh) {
            std::pair<key_tp, val_tp> node;
            node.first = *it;
            node.second = degree;
            results.push_back(node);
        }
    }
}

void DetectorSS::Reset() {
}


unsigned char** DetectorSS::GetTable() {
    return ss_.counts;
}

key_tp** DetectorSS::GetKey() {
    return ss_.skey;
}

int** DetectorSS::GetLevel() {
    return ss_.level;
}

//Union bmp2 to bmp1
//assume bmp1 and bmp2 have the same offset
void DetectorSS::Mergebmp(unsigned char* bmp1, unsigned char* bmp2,
        int start, int len) {

    int curlen = 0;
    while (curlen < len) {
        unsigned char temp = 0;
        int n = start + curlen;
        int index = n >> 3;
        int pos = n&0x00000007;

        temp = bmp2[index] & (0xff >> pos);
        bmp1[index] |= temp;
        curlen += 8-pos;
    }
}

void DetectorSS::Merge(DetectorSS *detector) {
    DetectorSS* ss = (DetectorSS*) detector;
    unsigned char** counts  = ss->GetTable();
    key_tp** skey = ss->GetKey();
    int** level =  ss->GetLevel();
    for (int i = 0; i < ss_.depth; i++) {
        for (int j = 0; j < ss_.width; j++) {
            Mergebmp(ss_.counts[i], counts[i], j*ss_.offsets[ss_.c], ss_.offsets[ss_.c]);
            if (ss_.level[i][j] <= level[i][j]) {
                ss_.skey[i][j] = skey[i][j];
                ss_.level[i][j] = level[i][j];
            }
        }
    }
}
