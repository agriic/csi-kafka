#include "producer.h"
#include <boost/thread.hpp>
#include <boost/bind.hpp>

namespace csi
{
    namespace kafka
    {
        producer::producer(boost::asio::io_service& io_service, const boost::asio::ip::tcp::resolver::query& query, const std::string& topic, int32_t partition) :
            _ios(io_service),
            _client(io_service, query),
            _topic_name(topic),
            _partition_id(partition)
        {
        }

        void producer::connect_async(connect_callback cb)
        {
            _client.connect_async(cb);
        }

        boost::system::error_code producer::connect()
        {
            return _client.connect();
        }

        void producer::send_async(int32_t required_acks, int32_t timeout, const std::vector<basic_message>& v, int32_t correlation_id, callback cb)
        {
            _client.perform_async(csi::kafka::create_produce_request(_topic_name, _partition_id, required_acks, timeout, v, correlation_id), [this, cb, correlation_id](const boost::system::error_code& ec, csi::kafka::basic_call_context::handle handle)
            {
                // find the error code thast belongs to this topic
                std::shared_ptr<produce_response>  result = parse_produce_response(handle);
                csi::kafka::error_codes error = csi::kafka::NoError;

                assert(correlation_id == result->correlation_id);

                //for now only one topic / many partitions
                assert(result->topics.size() == 1);
                for (std::vector<produce_response::topic_data>::const_iterator i = result->topics.begin(); i != result->topics.end(); ++i)
                {
                    if (i->topic_name != _topic_name)
                    {
                        std::cerr << "unexpected topic name " << i->topic_name << std::endl;
                        break;
                    }

                    // for now our call sends only one partition - so this will be a vector of 1
                    assert(i->partitions.size() == 1);
                    for (std::vector<produce_response::topic_data::partition_data>::const_iterator j = i->partitions.begin(); j != i->partitions.end(); ++j)
                    {
                        if (j->error_code())
                        {
                            std::cerr << "produce_response: topic:" << i->topic_name << ", partition:" << j->partition_id << ", error:"  << csi::kafka::to_string(j->error_code()) << std::endl;
                            error = j->error_code();
                        }
                    }
                }
                cb(error, result);
            });
        }
    } // kafka
}; // csi