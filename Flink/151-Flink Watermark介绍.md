### watermark 策略与应用     

#### Watermark 策略简介 
时间戳的分配与 watermark 的生成是齐头并进的，其可以告诉 Flink 应用程序事件时间的进度。其可以通过指定 `WatermarkGenerator` 来配置 watermark 的生成方式;  

使用 Flink API 时需要设置一个同时包含 TimestampAssigner 和 WatermarkGenerator 的 WatermarkStrategy。WatermarkStrategy 工具类中也提供了许多常用的 watermark 策略，并且用户也可以在某些必要场景下构建自己的 watermark 策略。     








refer   
1.https://nightlies.apache.org/flink/flink-docs-release-1.18/docs/dev/datastream/event-time/generating_watermarks/      
2.https://www.cnblogs.com/liugp/p/16255682.html#5watermarkstrategy%E9%87%8D%E7%82%B9        

