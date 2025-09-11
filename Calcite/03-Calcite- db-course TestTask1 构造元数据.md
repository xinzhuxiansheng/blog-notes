

## 


ParquetTable#convertParquetTypeToSqlType() 

定义字段
```java
Object[][] types = new Object[][] {
        {"VendorID", SqlTypeName.INTEGER, 10},
        {"tpep_pickup_datetime", SqlTypeName.TIMESTAMP, 6},
        {"tpep_dropoff_datetime", SqlTypeName.TIMESTAMP, 6},
        {"passenger_count", SqlTypeName.BIGINT, 19},
        {"trip_distance", SqlTypeName.DOUBLE, 15},
        {"RatecodeID", SqlTypeName.BIGINT, 19},
        {"store_and_fwd_flag", SqlTypeName.VARCHAR, -1},
        {"PULocationID", SqlTypeName.INTEGER, 10},
        {"DOLocationID", SqlTypeName.INTEGER, 10},
        {"payment_type", SqlTypeName.BIGINT, 19}, 
        {"fare_amount", SqlTypeName.DOUBLE, 15},
        {"extra", SqlTypeName.DOUBLE, 15},
        {"mta_tax", SqlTypeName.DOUBLE, 15},
        {"tip_amount", SqlTypeName.DOUBLE, 15},
        {"tolls_amount", SqlTypeName.DOUBLE, 15},
        {"improvement_surcharge", SqlTypeName.DOUBLE, 15},
        {"total_amount", SqlTypeName.DOUBLE, 15},
        {"congestion_surcharge", SqlTypeName.DOUBLE, 15},
        {"Airport_fee", SqlTypeName.DOUBLE, 15},
};
```  

名称，类型，精度，是否可为 NULL，经测试，INTEGER
```java
for (int f = 0; f < rowType.getFieldCount(); f++) {
    var field = rowType.getFieldList().get(f);

    assertThat(field.getName()).isEqualTo(types[f][0]);
    assertThat(field.getType().getSqlTypeName()).isEqualTo(types[f][1]);
    assertThat(field.getType().getPrecision()).isEqualTo(types[f][2]);
    assertThat(field.getType().isNullable()).isEqualTo(true);
}
```