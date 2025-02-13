#ifndef KCL_HPP
#define KCL_HPP

#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <librdkafka/rdkafka.h>

namespace KCL {
	// Structure representing a Kafka message
	struct Message {
	    std::string sender;
	    std::string content;
	    rd_kafka_message_t* rkmessage = nullptr;
	    rd_kafka_t* rk = nullptr;
	    bool consumed = false;
	    bool committed = false;
	};
	
	class Comm {
		private:
			// Kafka configuration
			static std::string brokers;
			static std::string indexTopic;
			static std::string processName;
			static std::string myTopic;
			static std::string group_id;

			// Thread control
			static bool stopThreads;
			static std::thread indexThread;
			static std::thread messagesThread;

			// Map to store process-to-topic associations
			static std::unordered_map<std::string, std::string> processTopicMap;
			static std::mutex processTopicMapMutex;

			// Queue for storing received messages
			static std::queue<Message> messageQueue;
			static std::mutex messageQueueMutex;
			static std::condition_variable messageQueueCondVar;

			// Kafka timeouts
			static int consumer_poll_timeout_ms;
			static int flush_timeout_ms;

			Comm() {}

			// Consumer for index topic, updates process-topic mapping
			static int consumerIndex() {
			        char errstr[512];
			        rd_kafka_resp_err_t err;

			        // Create Kafka consumer configuration
			        rd_kafka_conf_t* conf = rd_kafka_conf_new();
			        if (rd_kafka_conf_set(conf, "bootstrap.servers", brokers.c_str(), errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
			        	std::cerr << "rd_kafka_conf_new(): " << errstr << std::endl;
			            return 1;
			        }
			        
			        if (rd_kafka_conf_set(conf, "group.id", group_id.c_str(), errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
			        	std::cerr << "rd_kafka_conf_set(): " << errstr << std::endl;
			            return 1;
			        }
			        
			        if (rd_kafka_conf_set(conf, "auto.offset.reset", "earliest", errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
			        	std::cerr << "rd_kafka_conf_set(): " << errstr << std::endl;
			            return 1;
			        }

			        if (rd_kafka_conf_set(conf, "enable.auto.commit", "false", errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
			        	std::cerr << "rd_kafka_conf_set(): " << errstr << std::endl;
			        	return 1;
			        }

			        // Create Kafka consumer
			        rd_kafka_t* consumer_from_index = rd_kafka_new(RD_KAFKA_CONSUMER, conf, errstr, sizeof(errstr));
			        if (!consumer_from_index) {
			        	std::cerr << "rd_kafka_new(): " << errstr << std::endl;
			            return 1;
			        }

			        // Subscribe to index topic
			        rd_kafka_topic_partition_list_t* partitions = rd_kafka_topic_partition_list_new(1);
			        rd_kafka_topic_partition_list_add(partitions, indexTopic.c_str(), 0)->offset = RD_KAFKA_OFFSET_BEGINNING;
			        
			        if ((err = rd_kafka_assign(consumer_from_index, partitions)) != RD_KAFKA_RESP_ERR_NO_ERROR) {
			        	std::cerr << "rd_kafka_assign(): " << rd_kafka_err2str(err) << std::endl;
			            rd_kafka_topic_partition_list_destroy(partitions);
			            rd_kafka_destroy(consumer_from_index);
			            return 1;
			        }
			        
			       	rd_kafka_topic_partition_list_destroy(partitions);

			        // Poll messages and update process-topic map
			        while (!stopThreads) {
			            rd_kafka_message_t* message = rd_kafka_consumer_poll(consumer_from_index, consumer_poll_timeout_ms);
			        
			            if (message) {
			            	if (message->err) {
			                	if (message->err != RD_KAFKA_RESP_ERR__PARTITION_EOF && message->err != RD_KAFKA_RESP_ERR_UNKNOWN_TOPIC_OR_PART) {
			                    	std::cerr << "rd_kafka_consumer_poll(): " << rd_kafka_message_errstr(message) << std::endl;
			                    }
			                } else {
			                	std::lock_guard<std::mutex> lock(processTopicMapMutex);
			                	processTopicMap[std::string((char*)message->payload, message->len)] = std::string((char*)message->key, message->key_len);  
			                }

			                rd_kafka_message_destroy(message);
			            }
			        }

			        // Cleanup
			        rd_kafka_consumer_close(consumer_from_index);
			        rd_kafka_destroy(consumer_from_index);
			       	return 0;         
			}

			// Consumer for process messages
			static int consumerMessages() {
				char errstr[512];
				rd_kafka_resp_err_t err;

				// Create Kafka consumer configuration			        
				rd_kafka_conf_t* conf = rd_kafka_conf_new();
				if (rd_kafka_conf_set(conf, "bootstrap.servers", brokers.c_str(), errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
					std::cerr << "rd_kafka_conf_new(): " << errstr << std::endl;
					return 1;
				}
							        
				if (rd_kafka_conf_set(conf, "group.id", group_id.c_str(), errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
					std::cerr << "rd_kafka_conf_set(): " << errstr << std::endl;
					return 1;
				}
							        
				if (rd_kafka_conf_set(conf, "auto.offset.reset", "earliest", errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
					std::cerr << "rd_kafka_conf_set(): " << errstr << std::endl;
					return 1;
				}

				if (rd_kafka_conf_set(conf, "enable.auto.commit", "false", errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
					std::cerr << "rd_kafka_conf_set(): " << errstr << std::endl;
					return 1;
				}

				// Create Kafka consumer		        
				rd_kafka_t* consumer_from_messages = rd_kafka_new(RD_KAFKA_CONSUMER, conf, errstr, sizeof(errstr));
				if (!consumer_from_messages) {
					std::cerr << "rd_kafka_new(): " << errstr << std::endl;
					return 1;
				}

				// Subscribe to myTopic topic
				rd_kafka_topic_partition_list_t* topics = rd_kafka_topic_partition_list_new(1);
				rd_kafka_topic_partition_list_add(topics, myTopic.c_str(), RD_KAFKA_PARTITION_UA);
				
				if ((err = rd_kafka_subscribe(consumer_from_messages, topics)) != RD_KAFKA_RESP_ERR_NO_ERROR) {
					std::cerr << "rd_kafka_subscribe(): " << rd_kafka_err2str(err) << std::endl;
				    rd_kafka_topic_partition_list_destroy(topics);
				    rd_kafka_destroy(consumer_from_messages);
				    return 1;
				}

				rd_kafka_topic_partition_list_destroy(topics);

				// Poll messages and add in the internal queue
				while (!stopThreads) {
					rd_kafka_message_t* message = rd_kafka_consumer_poll(consumer_from_messages, consumer_poll_timeout_ms);
					
					if (message) {
						if (message->err) {
							if (message->err != RD_KAFKA_RESP_ERR__PARTITION_EOF && message->err != RD_KAFKA_RESP_ERR_UNKNOWN_TOPIC_OR_PART) {
								std::cerr << "rd_kafka_consumer_poll(): " << rd_kafka_message_errstr(message) << std::endl;
							}

							rd_kafka_message_destroy(message);
						} else {
							std::lock_guard<std::mutex> lock(messageQueueMutex);

							#ifdef AT_MOST_ONCE
								Message msg{std::string((char*)message->key, message->key_len),
											std::string((char*)message->payload, message->len)};

								rd_kafka_commit_message(consumer_from_messages, message, 0);
								rd_kafka_message_destroy(message);
							#else // AT_LEAST_ONCE
								Message msg{std::string((char*)message->key, message->key_len),
											std::string((char*)message->payload, message->len),
											message,
											consumer_from_messages};
							#endif

							messageQueue.push(msg);
							messageQueueCondVar.notify_all();    		
						}
					}
				}

				// Cleanup	        
				rd_kafka_consumer_close(consumer_from_messages);
				rd_kafka_destroy(consumer_from_messages);		        
				return 0;
			}

			// Checks if a message from a specific sender exists
			static bool hasMessageFrom(const std::string& sender) {
				std::queue<Message> tempQueue = messageQueue;
				
        		while (!tempQueue.empty()) {
        			#ifdef AT_MOST_ONCE
        				if (tempQueue.front().sender == sender)
        					return true;
					#else // AT_LEAST_ONCE
            			if (tempQueue.front().sender == sender && !tempQueue.front().consumed)
                			return true;
            		#endif
            		
            		tempQueue.pop();
        		}
        		
        		return false;
    		}

			// Discards and commits all stored messages (AT_LEAST_ONCE)
    		static void discardAndCommitAllMessages() {
    			std::lock_guard<std::mutex> lock(messageQueueMutex);
    			
    			std::vector<Message> messages;

    			while (!messageQueue.empty()) {
    				messages.push_back(std::move(messageQueue.front()));
    			    messageQueue.pop();
    			}

    			Message* commitCandidate = nullptr;

    			for (auto &msg : messages) {
    				if (msg.consumed)
    			    	commitCandidate = &msg;
    			    else
    			    	break; 
    			}
    			
    			if (commitCandidate && !commitCandidate->committed)
    				rd_kafka_commit_message(commitCandidate->rk, commitCandidate->rkmessage, 0);
    			
    			
    			for (auto &msg : messages)
    				rd_kafka_message_destroy(msg.rkmessage);
    		}
		
		public:
			// Initializes the communication module
			static void Init(std::string processName) {
				Comm::processName = processName;
				myTopic = processName + "_topic";
				group_id = processName;
				indexThread = std::thread(consumerIndex);
			}

			// Cleans up and stops the module
			static void Finalize() {
				stopThreads = true;

				#ifndef AT_MOST_ONCE
					discardAndCommitAllMessages();
				#endif
				
				if (indexThread.joinable())
					indexThread.join();
				
				if (messagesThread.joinable())
					messagesThread.join();
			}

			// Starts listening for messages
			static int Listen() {
				char errstr[512];

				// Create Kafka producer configuration
				rd_kafka_conf_t* conf = rd_kafka_conf_new();
				if (rd_kafka_conf_set(conf, "bootstrap.servers", brokers.c_str(), errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
					std::cerr << "rd_kafka_conf_set(): " << errstr << std::endl;
				    return 1;
				}

				// Create Kafka producer
				rd_kafka_t* producer_to_index = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));
				if (!producer_to_index) {
					std::cerr << "rd_kafka_new(): " << errstr << std::endl;
				    return 1;
				}

				// Create myTopic topic
				rd_kafka_topic_t* rkt_myTopic = rd_kafka_topic_new(producer_to_index, myTopic.c_str(), NULL);
				if (!rkt_myTopic) {
					std::cerr << "rd_kafka_topic_new(): " << rd_kafka_err2str(rd_kafka_last_error()) << std::endl;
				    rd_kafka_destroy(producer_to_index);
				   	return 1;
				}

				// Create index topic
				rd_kafka_topic_t* rkt_indexTopic = rd_kafka_topic_new(producer_to_index, indexTopic.c_str(), NULL);
				if (!rkt_indexTopic) {
					std::cerr << "rd_kafka_topic_new(): " << rd_kafka_err2str(rd_kafka_last_error()) << std::endl;
					rd_kafka_destroy(producer_to_index);
					return 1;
				}

				// Publish association (processName, myTopic) to index topic
				if (rd_kafka_produce(
					rkt_indexTopic,
				    RD_KAFKA_PARTITION_UA,
				    RD_KAFKA_MSG_F_COPY,
				    (void*)processName.c_str(),
				    processName.size(),
				    (void*)myTopic.c_str(),
				    myTopic.size(),
				    NULL) == -1) {
				    	std::cerr << "rd_kafka_produce(): " << rd_kafka_err2str(rd_kafka_last_error()) << std::endl;
				}
				
				rd_kafka_poll(producer_to_index, 0);

				// Cleanup
				rd_kafka_flush(producer_to_index, flush_timeout_ms);
				rd_kafka_topic_destroy(rkt_myTopic);
				rd_kafka_topic_destroy(rkt_indexTopic);
				rd_kafka_destroy(producer_to_index);

				// Start consumer thread for messages
				messagesThread = std::thread(consumerMessages);

				return 0;
			}

			// Sends a message to a specific process
			static int Send(const std::string& targetProcess, const std::string& messageContent) {
				std::string targetTopic;

				// Find the target topic
				{
					std::lock_guard<std::mutex> lock(processTopicMapMutex);

					if (auto topic = processTopicMap.find(targetProcess); topic != processTopicMap.end())
						targetTopic = topic->second;
					else
						return 2;
				}

				char errstr[512];

				// Create Kafka producer configuration
				rd_kafka_conf_t* conf = rd_kafka_conf_new();
				if (rd_kafka_conf_set(conf, "bootstrap.servers", brokers.c_str(), errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
					std::cerr << "rd_kafka_conf_set(): " << errstr << std::endl;
					return 1;
				}

				// Create Kafka producer	
				rd_kafka_t* producer_send = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));
				if (!producer_send) {
					std::cerr << "rd_kafka_new(): " << errstr << std::endl;
					return 1;
				}

				// Create target topic	
				rd_kafka_topic_t* rkt_targetTopic = rd_kafka_topic_new(producer_send, targetTopic.c_str(), NULL);
				if (!rkt_targetTopic) {
					std::cerr << "rd_kafka_topic_new(): " << rd_kafka_err2str(rd_kafka_last_error()) << std::endl;
					rd_kafka_destroy(producer_send);
					return 1;
				}

				// Send the message
				if (rd_kafka_produce(
					rkt_targetTopic,
					RD_KAFKA_PARTITION_UA,
					RD_KAFKA_MSG_F_COPY,
					(void*)messageContent.c_str(),
					messageContent.size(),
					(void*)processName.c_str(),
					processName.size(),
					NULL) == -1) {
						std::cerr << "rd_kafka_produce(): " << rd_kafka_err2str(rd_kafka_last_error()) << std::endl;
				}
								
				rd_kafka_poll(producer_send, 0);

				// Cleanup			
				rd_kafka_flush(producer_send, flush_timeout_ms);
				rd_kafka_topic_destroy(rkt_targetTopic);
				rd_kafka_destroy(producer_send);
				
				return 0;	
			} 

			// Receives a message from a specific process
			static void Receive(const std::string& targetProcess, std::string& buff) {
        		std::unique_lock<std::mutex> lock(messageQueueMutex);
        		messageQueueCondVar.wait(lock, [&]() { return hasMessageFrom(targetProcess); });
        		        		
        		Message first_message;
        		std::queue<Message> tempQueue;

        		#ifdef AT_MOST_ONCE
        			while (!messageQueue.empty()) {
        				Message msg = std::move(messageQueue.front());
        			    messageQueue.pop();
        			        		        			
        			    if (first_message.sender.empty() && msg.sender == targetProcess)
        			    	first_message = msg;
        			    else
        			        tempQueue.push(std::move(msg));
        			}
        		#else // AT_LEAST_ONCE
        			bool commit = true;
        		        		        		
        		    while (!messageQueue.empty()) {
        		    	Message msg = std::move(messageQueue.front());
        		        messageQueue.pop();
        		        		        			
        		        if (first_message.sender.empty() && msg.sender == targetProcess && !msg.consumed) {
        		          	first_message = msg;
        		           	first_message.consumed = true;
        		
        		        	if (commit) {
        		        		rd_kafka_commit_message(first_message.rk, first_message.rkmessage, 0);
        		        		first_message.committed = true;
        		        	}
        		
        		        	tempQueue.push(first_message);
        		        } else {
        		        	commit &= msg.consumed;
        		        	tempQueue.push(std::move(msg));
        		       	}
        		    }
        		#endif
        		
        		messageQueue = std::move(tempQueue);
        		buff = first_message.content;
			}
	};

	// Static member initialization
	bool Comm::stopThreads = false;
	std::string Comm::group_id;
	std::string Comm::processName;
	std::string Comm::myTopic;
	std::thread Comm::indexThread;
	std::thread Comm::messagesThread;
	std::unordered_map<std::string, std::string> Comm::processTopicMap;
	std::mutex Comm::processTopicMapMutex;
	std::queue<Message> Comm::messageQueue;
	std::mutex Comm::messageQueueMutex;
	std::condition_variable Comm::messageQueueCondVar;

	// ---- CONFIGURATION SETTINGS ----
	std::string Comm::brokers = "localhost:9092";
	std::string Comm::indexTopic = "index";
	int Comm::consumer_poll_timeout_ms = 500;
	int Comm::flush_timeout_ms = 5000;
};

#endif // KCL_HPP
