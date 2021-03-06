#include <boost/thread.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <csi_kafka/kafka.h>
#include <csi_kafka/highlevel_consumer.h>

#define TOPIC_NAME     "csi-kafka.test-topic"
#define DEFAULT_BROKER_ADDRESS "52.0.43.130"
/*
    to get this working you have to prepare the broker
    1) first make sure the topic is existing
    2) create a path in zookeeper
    see README.txt
*/


void handle_fetch(csi::kafka::highlevel_consumer* consumer, std::vector<csi::kafka::rpc_result<csi::kafka::fetch_response>> response)
{
    for (std::vector<csi::kafka::rpc_result<csi::kafka::fetch_response>>::const_iterator i = response.begin(); i != response.end(); ++i)
    {
        if (i->ec)
        {
            std::cerr << "error: " << to_string(i->ec) << std::endl;
            consumer->close();
            return;
        }

        for (std::vector<csi::kafka::fetch_response::topic_data>::const_iterator j = (*i)->topics.begin(); j != (*i)->topics.end(); ++j)
        {
            for (std::vector<std::shared_ptr<csi::kafka::fetch_response::topic_data::partition_data>>::const_iterator k = j->partitions.begin(); k != j->partitions.end(); ++k)
            {
                if ((*k)->error_code)
                {
                    std::cerr << "error: " << to_string((csi::kafka::error_codes) (*k)->error_code) << std::endl;
                    consumer->close();
                    continue;
                }
                int64_t last_index = -1;
                for (std::vector<std::shared_ptr<csi::kafka::basic_message>>::const_iterator l = (*k)->messages.begin(); l != (*k)->messages.end(); ++l)
                {

                }
                if ((*k)->messages.size())
                    std::cerr << "partition: " << (*k)->partition_id << ", lag: " << (*k)->highwater_mark_offset - (*((*k)->messages.end() - 1))->offset << " msg" << std::endl;
            }
        }
    }
    consumer->fetch(boost::bind(handle_fetch, consumer, _1));
}


int main(int argc, char** argv)
{
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
    int32_t port = (argc >= 3) ? atoi(argv[2]) : 9092;

    csi::kafka::broker_address broker;
    if (argc >= 2)
    {
        broker = csi::kafka::broker_address(argv[1], port);
    }
    else
    {
        broker = csi::kafka::broker_address(DEFAULT_BROKER_ADDRESS, port);
    }

    boost::asio::io_service io_service;
    std::auto_ptr<boost::asio::io_service::work> work(new boost::asio::io_service::work(io_service));
    boost::thread bt(boost::bind(&boost::asio::io_service::run, &io_service));

 


    //HIGHLEVEL CONSUMER
    csi::kafka::highlevel_consumer consumer(io_service, TOPIC_NAME, 1000, 10000);
    consumer.connect( { broker } );
    //std::vector<int64_t> result = consumer.get_offsets();
    consumer.connect_forever({ broker } );

    consumer.set_offset(csi::kafka::earliest_available_offset);

    boost::thread do_log([&consumer]
    {
        boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::rolling_mean> > acc(boost::accumulators::tag::rolling_window::window_size = 10);
        while (true)
        {
            boost::this_thread::sleep(boost::posix_time::seconds(1));

            std::vector<csi::kafka::highlevel_consumer::metrics>  metrics = consumer.get_metrics();
            uint32_t rx_msg_sec_total = 0;
            uint32_t rx_kb_sec_total = 0;
            for (std::vector<csi::kafka::highlevel_consumer::metrics>::const_iterator i = metrics.begin(); i != metrics.end(); ++i)
            {
                std::cerr << "\t partiton:" << (*i).partition << "\t" << (*i).rx_msg_sec << " msg/s \t" << (*i).rx_kb_sec << "KB/s \troundtrip:" << (*i).rx_roundtrip << " ms" << std::endl;
                rx_msg_sec_total += (*i).rx_msg_sec;
                rx_kb_sec_total += (*i).rx_kb_sec;
            }
            std::cerr << "\t          \t" << rx_msg_sec_total << " msg/s \t" << rx_kb_sec_total << "KB/s" << std::endl;
        }
    });


    consumer.stream_async([](const boost::system::error_code& ec1, csi::kafka::error_codes ec2, std::shared_ptr<csi::kafka::fetch_response::topic_data::partition_data> response)
    {
        if (ec1 || ec2)
        {
            std::cerr << " decode error: ec1:" << ec1 << " ec2" << csi::kafka::to_string(ec2) << std::endl;
            return;
        }
        std::cerr << "+";
    });

    consumer.fetch(boost::bind(handle_fetch, &consumer, _1));


    while (true)
        boost::this_thread::sleep(boost::posix_time::seconds(30));

    consumer.close();

    work.reset();
    io_service.stop();

    return EXIT_SUCCESS;
}
