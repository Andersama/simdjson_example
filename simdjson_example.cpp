// simdjson_example.cpp : Defines the entry point for the application.
//
#include <iostream>
#include <string>
#include "simdjson.h"

std::string mangled_example_json = R"raw(        {
          "t": "Temp Diff",
          "xvy": 0,
          "pd": 60,
          "sz": 4,
          "l": [
                "dFarenheit",
                "sum",
                "Low",
                "High"
          ],
          "c": [
                "green",
                "orange",
                "cyan",
                "yellow"
          ],
          "d": [
                0,
                0,
                0.5,
                10
          ]
        },
        {
          "t": "Power",
          "xvy": 0,
          "pd": 60,
          "sz": 3,
          "l": [
                "watts (est. ligth)",
                "watts (1.9~gpm)",
                "watts (1gpm)"
          ],
          "c": [
                "green",
                "orange",
                "cyan"
          ],
          "d": [
                181.78063964,
                114.88922882,
                59.35943603
          ]
        }
  ]
}   !@#$%^&*()`;'?><.0123456789some_errant_data   {"t": 5649,
  "ng": 5,
  "lu": 4086,
  "g": [
        {
          "t": "Temps",
          "xvy": 0,
          "pd": 60,
          "sz": 3,
          "l": [
                "Low Temp",
                "High Temp",
                "Smooth Avg"
          ],
          "c": [
                "green",
                "orange",
                "cyan"
          ],
          "d": [
                80.48750305,
                80.82499694,
                80.65625
          ]
        },
        {
          "t": "Pump Status",
          "xvy": 0,
          "pd": 60,
          "sz": 3,
          "l": [
                "Pump",
                "Thermal",
                "Light"
          ],
          "c": [
                "green",
                "orange",
                "cyan"
          ],
          "d": [
                0,
                0,
                0
          ]
        },
        {
          "t": "Lux",
          "xvy": 0,
          "pd": 60,
          "sz": 4,
          "l": [
                "Value",
                "Smooth",
                "Low",
                "High"
          ],
          "c": [
                "green",
                "orange",
                "cyan",
                "yellow"
          ],
          "d": [
                2274.62939453,
                2277.45947265,
                4050,
                4500
          ]
        },
                      
                              
                               
                  )raw";

                
int main()
{
    std::string buffer;
    buffer.reserve(mangled_example_json.size()*4);

    simdjson::ondemand::parser parser;
    simdjson::ondemand::document json_document;

    //Rough idea, in a couple loops appending the mangled_example_json above forms a complete object
    //although with some garbage inbetween. Here we're aiming to slowly chew through the data crossing
    //fingers we find / land on a { that's the begining of the complete object somewhere in the stream

    //bit leaky this loop doesn't keep up with what we're appending
    while(true) {
        //append to the existing buffer
        if (buffer.capacity() < (buffer.size() + mangled_example_json.size() + simdjson::SIMDJSON_PADDING)) {
            buffer.reserve(((buffer.size() + mangled_example_json.size()) * 2) + simdjson::SIMDJSON_PADDING);
        }
        buffer.append(mangled_example_json.data(), mangled_example_json.size());

        //find a rough spot to parse from
        auto lbrace = std::find(buffer.data(), buffer.data()+buffer.size(), '{');
        size_t lbrace_distance = lbrace - buffer.data();
        simdjson::padded_string_view potential_json(buffer.data()+lbrace_distance,buffer.size()-lbrace_distance,buffer.capacity()-lbrace_distance);
        
        //attempt at a parse
        auto doc_error = parser.iterate(potential_json).get(json_document);
        if (!doc_error) {
            std::string_view v;
/*
            //this works although this seems redundant, I've no idea why it does
            auto json_error = simdjson::to_json_string(json_document).get(v);
            if (json_error == simdjson::INCOMPLETE_ARRAY_OR_OBJECT) {
                std::cout << "incomplete object\n";
                if (lbrace_distance)
                    buffer.erase(size_t{0},lbrace_distance);
                continue;
            } else if (json_error != simdjson::SUCCESS) {
                std::cout << "malformed object\n";
                buffer.erase(buffer.begin());
                continue;
            }
*/
/*
            //this gets stuck in incomplete_array_or_object, seems like it should
            //behave like the lines above
            simdjson::ondemand::object obj;
            auto objerr = json_document.get_object().get(obj);
            if (objerr == simdjson::INCOMPLETE_ARRAY_OR_OBJECT) {
                //nothing to parse here yet...
                std::cout << "incomplete object\n";
                if (lbrace_distance)
                    buffer.erase(size_t{0},lbrace_distance);
                continue;
            } else if (objerr != simdjson::SUCCESS) {
                //panic because somethings gone horribly wrong...
                std::cout << "malformed object\n";
                buffer.erase(buffer.begin());
                continue;
            } else {
                //ok?
                v = {buffer.data()+lbrace_distance, 1}; //stream_buffer.capacity()-dist
            }
            */
            /*
            //never finds the t field, although it'll validate we're on an object
            simdjson::ondemand::json_type jtype;
            auto type_err = json_document.type().get(jtype);
            if (type_err) {
                std::cout << "failed to get document type\n";
                buffer.erase(buffer.begin());
            }
            if (jtype != simdjson::ondemand::json_type::object) {
                std::cout << "not an object\n";
                buffer.erase(buffer.begin());
            }
            */
            //doing none of the above? we'll never find "t" field
            //top level of the example object is a timestamp in milliseconds
            size_t ts;
            //auto t_err = json_document["t"].get_uint64().get(ts);
            auto t_err = json_document["t"].get(ts); //both this and the line above appear to do the same thing?

            //and unfortunately I've failed to replicate the error (again). It seems to be intermittant, although
            //these three patterns were roughly what I was doing before
            if (t_err) {
                //may be a valid json object, but not one we're interested in
                std::cout << "could not find t field\n";
                auto json_error = simdjson::to_json_string(json_document).get(v);
                if (json_error != simdjson::SUCCESS) {
                    buffer.erase(buffer.begin());
                    continue;
                } else {
                    //clear everything up to the end of the json object
                    buffer.erase(size_t{0}, (v.data() + v.size()) - buffer.data());
                    continue;
                }
            } else {
                //we should be fairly confident to parse as normal...
                //g is another field we're interested in, if it's not here, it's a json object
                //but not one we're interested in
                simdjson::ondemand::array g_array;
                auto g_err = json_document["g"].get_array().get(g_array);
                if (g_err) {
                    std::cout << "could not find g field\n";
                    auto json_error = simdjson::to_json_string(json_document).get(v);
                    if (json_error != simdjson::SUCCESS) {
                        buffer.erase(buffer.begin());
                        continue;
                    } else {
                        //clear everything up to the end of the json object
                        buffer.erase(size_t{0}, (v.data() + v.size()) - buffer.data());
                        continue;
                    }
                } else {
                    //could write this all with error checking, but I'm not going to specificially because it's not
                    //this part I'm having problems with...
                    size_t count = 0;
                    for (auto item : g_array) {
                        std::cout << "item: " << count << '\n';
                        count++;
                        std::string_view item_v;
                        auto item_error = simdjson::to_json_string(item).get(item_v);
                        if (item_error) {
                        } else {
                            //bit bugged, we're trying to reconstruct what the working version does with v to start with
                            //we're going to be short the ending }
                            v = {buffer.data()+lbrace_distance, (size_t)((item_v.data()+item_v.size()) - (buffer.data()+lbrace_distance))};
                        }
                    }
                }
                //clear everything up to the end of the json object...
                //to prove this loop's error tolerant, we're not going to properly erase the entire object
                //we'll be missing the last } (which will start the next cycle of the stream in an odd position)
                buffer.erase(size_t{0}, (v.data() + v.size()) - buffer.data());
                std::cout << v << "}\n";
            }
        }
    }

	return 0;
}
