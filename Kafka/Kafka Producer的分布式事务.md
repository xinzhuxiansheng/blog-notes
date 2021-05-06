--In Blog
--Tags: Kafka

# Kafka Producer的分布式事务

>涉及Kafka是2.2.1版本



i will first briefly talk about exactly once up guarantees,
for stream processing in kafka,
and how it was achieved with a kafka transactions.
Then I will discuss some of the existing limits,
we've ovserved with original design.
Boyang then will provide out approaches,
pushing those limits,
to make exactly once more scalable and useful.
And after that,we'd love to hear your thoughts
and take any questions you have.
So,the exactly once semantics.
Let's take our time back in 2017.
At that time,we will see many users adopting kafka,
as a centralized streaming backbone for many of their real time applications,
and it has been a become a common practice trying to make sure that applications processing correctness is always guaranteed.
Even if a processor has failed.






Kafka Transactions