## docker-compose 部署Prometheus PushGateway Grafana    

### docker-compose.yml  
```yml
version: '3'
services:
  prometheus:
    image: prom/prometheus:v2.29.0
    volumes:
      - ./prometheus.yml:/etc/prometheus/prometheus.yml
    command:
      - '--config.file=/etc/prometheus/prometheus.yml'
    ports:
      - 9090:9090
    networks:
      - monitoring
  
  pushgateway:
    image: prom/pushgateway:v1.4.0
    ports:
      - 9091:9091
    networks:
      - monitoring
  
  grafana:
    image: grafana/grafana:8.0.6
    ports:
      - 3000:3000
    networks:
      - monitoring
    environment:
      - GF_SECURITY_ADMIN_USER=admin
      - GF_SECURITY_ADMIN_PASSWORD=admin
    volumes:
      - /root/yzhou/prometheus/grafana-storage:/var/lib/grafana
  
networks:
  monitoring:
    driver: bridge

#volumes:
#  grafana-storage:
```

### prometheus.yml  
```yml
# my global config
global:
  scrape_interval: 15s # Set the scrape interval to every 15 seconds. Default is every 1 minute.
  evaluation_interval: 15s # Evaluate rules every 15 seconds. The default is every 1 minute.
  # scrape_timeout is set to the global default (10s).

# Alertmanager configuration
alerting:
  alertmanagers:
    - static_configs:
        - targets:
          # - alertmanager:9093

# Load rules once and periodically evaluate them according to the global 'evaluation_interval'.
rule_files:
  # - "first_rules.yml"
  # - "second_rules.yml"

# A scrape configuration containing exactly one endpoint to scrape:
# Here it's Prometheus itself.
scrape_configs:
  # The job name is added as a label `job=<job_name>` to any timeseries scraped from this config.
  - job_name: "prometheus"

    # metrics_path defaults to '/metrics'
    # scheme defaults to 'http'.

    static_configs:
      - targets: ["localhost:9090"]  
```

```shell
docker-compose -f docker-compose.yml up -d  
```