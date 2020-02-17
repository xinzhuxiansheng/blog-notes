## metrics项
`metrics项` 是Grafana 配置项

每项解释：

|   监控项  |   metrics项 | 说明 | 级别  |
| :-------- | --------:| ---- |:--: |
| source-record-poll-rate  | kafka_connect_source_task_metrics_source_record_poll_rate |     |     |
| byte_rate     |  kafka_connect_mirror_mirrorsourceconnector_byte_rate |  每秒平均同步的字节数  |     |
| record-count      |  kafka_connect_mirror_mirrorsourceconnector_record_count |   |    sum,detail |
| record-age-ms     |  kafka_connect_mirror_mirrorsourceconnector_record_age_ms |   |     |
| record-age-ms-min     |  kafka_connect_mirror_mirrorsourceconnector_record_age_ms_min |   |   当前值  |
| record-age-ms-max     |  kafka_connect_mirror_mirrorsourceconnector_record_age_ms_max |   |     |
| record-age-ms-avg     |  kafka_connect_mirror_mirrorsourceconnector_record_age_ms_avg | 复制到目标集群时传入源记录的平均年龄  |     |
| replication-latency-ms     |  kafka_connect_mirror_mirrorsourceconnector_replication_latency_ms |   |   当前值  |
| replication-latency-ms-min     |  kafka_connect_mirror_mirrorsourceconnector_replication_latency_ms_min |   |     |
| replication-latency-ms-max     |  kafka_connect_mirror_mirrorsourceconnector_replication_latency_ms_max |   |     |
| replication-latency-ms-avg     |  kafka_connect_mirror_mirrorsourceconnector_replication_latency_ms_avg |   |     |
| records-consumed-total     |  kafka_consumer_consumer_fetch_manager_metrics_records_consumed_total |   |     |
| task-count     |  kafka_connect_connect_worker_metrics_task_count | 当前worker 分配的任务数量  |   当前指  |
| connector-running-task-count     |  kafka_connect_connect_worker_metrics_connector_running_task_count | 当前worker 正在运行的任务数量   |     |
| total-retries     |  kafka_connect_task_error_metrics_total_retries |   |   差值  |
| source-record-active-count     |  kafka_connect_source_task_metrics_source_record_active_count |   |   没用  |
| source-record-active-count-max     |  kafka_connect_source_task_metrics_source_record_active_count_max |   |  没用   |
| source-record-active-count-avg     |  kafka_connect_source_task_metrics_source_record_active_count_avg |   |     |
| record-error-rate     |  kafka_producer_producer_topic_metrics_record_error_rate |   | client.id维度   当前值 |
| last-heartbeat-seconds-ago     |  kafka_connect_connect_coordinator_metrics_last_heartbeat_seconds_ago |   |  当前值   |
| connector-count     |  kafka_connect_connect_worker_metrics_connector_count |   |   当前值  |
| total-records-skipped     |  kafka_connect_task_error_metrics_total_records_skipped |   |    求差 |
| total-record-failures     |  kafka_connect_task_error_metrics_total_record_failures |  此任务中记录处理失败的次数 |     |
| record-error-rate     |  kafka_producer_producer_topic_metrics_record_error_rate |   |     topic维度|
| source-record-write-total     |  kafka_connect_source_task_metrics_source_record_write_total |   |   求差  |
| source-record-write-rate     |  kafka_connect_source_task_metrics_source_record_write_rate |   |  当前值   |
| offset-commit-avg-time-ms     |  kafka_connect_connector_task_metrics_offset_commit_avg_time_ms |   |   当前值  |
| offset-commit-success-percentage     |  kafka_connect_connector_task_metrics_offset_commit_success_percentage |   |   当前值  |
| offset-commit-failure-percentage     |  kafka_connect_connector_task_metrics_offset_commit_failure_percentage |   | 当前    |
| offset-commit-max-time-ms     |  kafka_connect_connector_task_metrics_offset_commit_max_time_ms |   |  当前值   |


