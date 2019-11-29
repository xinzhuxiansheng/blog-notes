

## 官网摘要

Balancing leadership
Whenever a broker stops or crashes leadership for that broker's partitions transfers to other replicas. This means that by default when the broker is restarted it will only be a follower for all its partitions, meaning it will not be used for client reads and writes.

To avoid this imbalance, Kafka has a notion of preferred replicas. If the list of replicas for a partition is 1,5,9 then node 1 is preferred as the leader to either node 5 or 9 because it is earlier in the replica list. You can have the Kafka cluster try to restore leadership to the restored replicas by running the command:

 > bin/kafka-preferred-replica-election.sh --zookeeper zk_host:port/chroot

Since running this command can be tedious you can also configure Kafka to do this automatically by setting the following configuration:

 > auto.leader.rebalance.enable=true


