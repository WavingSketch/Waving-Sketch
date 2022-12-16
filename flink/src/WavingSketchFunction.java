package wavingsketch;

import org.apache.flink.api.java.tuple.Tuple2;
import org.apache.flink.streaming.api.functions.KeyedProcessFunction;
import org.apache.flink.util.Collector;

public class WavingSketchFunction extends KeyedProcessFunction<Integer, Tuple2<Integer, Long>, Integer> {
    // use 1 MB memory by default, 68 is the size(in bytes) of a Waving Sketch's bucket.
    private final WavingSketch wavingSketch = new WavingSketch(8, 1000 * 1000 / 68);

    @Override
    public void processElement(
            Tuple2<Integer, Long> item,
            Context ctx,
            Collector<Integer> collector) {
        // process each element in the stream
        int id = item.f0;
        wavingSketch.insert(id);
    }
}
