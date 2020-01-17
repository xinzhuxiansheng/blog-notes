

## metrics项
`metrics项` 是Grafana 配置项


|   监控项  |   metrics项 | 说明 | 级别  |
| :-------- | --------:| ---- |:--: |
| source-record-poll-rate  | kafka_connect_source_task_metrics_source_record_poll_rate |     |     |
| byte_rate     |  kafka_connect_mirror_mirrorsourceconnector_byte_rate |    |     |
| record-count      |  kafka_connect_mirror_mirrorsourceconnector_record_count |   |     |
| record-age-ms     |  kafka_connect_mirror_mirrorsourceconnector_record_age_ms |   |     |
| record-age-ms-min     |  kafka_connect_mirror_mirrorsourceconnector_record_age_ms_min |   |     |
| record-age-ms-max     |  kafka_connect_mirror_mirrorsourceconnector_record_age_ms_max |   |     |
| record-age-ms-avg     |  kafka_connect_mirror_mirrorsourceconnector_record_age_ms_avg |   |     |
| replication-latency-ms     |  kafka_connect_mirror_mirrorsourceconnector_replication_latency_ms |   |     |
| replication-latency-ms-min     |  kafka_connect_mirror_mirrorsourceconnector_replication_latency_ms_min |   |     |
| replication-latency-ms-max     |  kafka_connect_mirror_mirrorsourceconnector_replication_latency_ms_max |   |     |
| replication-latency-ms-avg     |  kafka_connect_mirror_mirrorsourceconnector_replication_latency_ms_avg |   |     |
| records-consumed-total     |  kafka_consumer_consumer_fetch_manager_metrics_records_consumed_total |   |     |
| task-count     |  kafka_connect_connect_worker_metrics_task_count |   |     |
| connector-running-task-count     |  kafka_connect_connect_worker_metrics_connector_running_task_count |   |     |
| total-retries     |  kafka_connect_task_error_metrics_total_retries |   |     |
| source-record-active-count     |  kafka_connect_source_task_metrics_source_record_active_count |   |     |
| source-record-active-count-max     |  kafka_connect_source_task_metrics_source_record_active_count_max |   |     |
| source-record-active-count-avg     |  kafka_connect_source_task_metrics_source_record_active_count_avg |   |     |
| record-error-rate     |  kafka_producer_producer_topic_metrics_record_error_rate |   |     |
| last-heartbeat-seconds-ago     |  kafka_connect_connect_coordinator_metrics_last_heartbeat_seconds_ago |   |     |
| connector-count     |  kafka_connect_connect_worker_metrics_connector_count |   |     |
| total-records-skipped     |  kafka_connect_task_error_metrics_total_records_skipped |   |     |
| total-record-failures     |  kafka_connect_task_error_metrics_total_record_failures |   |     |
| record-error-rate     |  kafka_producer_producer_topic_metrics_record_error_rate |   |     |
| source-record-write-total     |  kafka_connect_source_task_metrics_source_record_write_total |   |     |
| source-record-write-rate     |  kafka_connect_source_task_metrics_source_record_write_rate |   |     |
| offset-commit-avg-time-ms     |  kafka_connect_connector_task_metrics_offset_commit_avg_time_ms |   |     |
| offset-commit-success-percentage     |  kafka_connect_connector_task_metrics_offset_commit_success_percentage |   |     |
| offset-commit-failure-percentage     |  kafka_connect_connector_task_metrics_offset_commit_failure_percentage |   |     |
| offset-commit-max-time-ms     |  kafka_connect_connector_task_metrics_offset_commit_max_time_ms |   |     |
| offset-commit-max-time-ms     |  kafka_connect_connector_task_metrics_offset_commit_max_time_ms |   |     |
| offset-commit-max-time-ms     |  kafka_connect_connector_task_metrics_offset_commit_max_time_ms |   |     |

