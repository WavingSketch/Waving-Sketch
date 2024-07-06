package wavingsketch;

import org.apache.flink.api.common.typeinfo.Types;
import org.apache.flink.api.java.tuple.Tuple2;
import org.apache.flink.api.java.utils.ParameterTool;
import org.apache.flink.streaming.api.datastream.DataStream;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;

public class WavingSketchJob {
    public static void main(String[] args) throws Exception {
        ParameterTool params = ParameterTool.fromArgs(args);

        StreamExecutionEnvironment env = StreamExecutionEnvironment.getExecutionEnvironment();

        DataStream<Tuple2<Integer, Long>> source = env
                .readTextFile(params.get("input"))
                .map(x -> {
                    String[] value = x.split(",");
                    return new Tuple2<>(Integer.parseUnsignedInt(value[0]), Long.parseUnsignedLong(value[1]));
                })
                .returns(Types.TUPLE(Types.INT, Types.LONG));

        DataStream<Integer> keyedStream = source
                .keyBy(value -> value.f0)
                .process(new WavingSketchFunction())
                .name("waving-sketch-function");

        keyedStream.print().setParallelism(1);

        env.execute("Waving Sketch Insertion");
    }
}
