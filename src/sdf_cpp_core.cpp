/**
 *  Copyright 2024 Niklas Meyer
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <string>
#include <nlohmann/json.hpp>
#include "sdf/sdf_cpp_core.h"

using json = nlohmann::ordered_json;

namespace sdf {

//! Reference to the SDF-Model used for resolving sdfRef-Elements.
json global_sdf_model = {};

//! Function used to resolve sdfRef qualities.
//! This function implements the procedure described in section 4.4 of the sdf specification.
void ResolveSdfRef(json& sdf_ref_qualities_json) {
    std::string sdf_ref = sdf_ref_qualities_json.at("sdfRef");
    // Create the patch variable
    json patch = sdf_ref_qualities_json;
    // Remove the sdfRef member in patch
    patch.erase("sdfRef");
    // Check if the model contains the target structure
    if (global_sdf_model.contains(json::json_pointer(sdf_ref.substr(1)))) {
        // Dereference the pointer
        json original = global_sdf_model.at(json::json_pointer(sdf_ref.substr(1)));
        // Apply the Merge Patch Algorithm [RFC7396]
        original.merge_patch(patch);
        // Use the result in place of the original JSON map
        sdf_ref_qualities_json = original;
    }
}

//! Function used to parse common qualities from json into a CommonQuality object.
void ParseCommonQualities(json& common_quality_json, CommonQuality& common_quality) {
    // If a sdfRef-Element exists, resolve it
    // Furthermore, set the fill the sdfRef field for later processing
    if (common_quality_json.contains("sdfRef")) {
        common_quality_json.at("sdfRef").get_to(common_quality.sdf_ref);
        // Additionally to setting sdfRef, we resolve sdfRef
        ResolveSdfRef(common_quality_json);
    }

    if (common_quality_json.contains("description")) {
        common_quality_json.at("description").get_to(common_quality.description);
    }

    if (common_quality_json.contains("label")) {
        common_quality_json.at("label").get_to(common_quality.label);
    }

    if (common_quality_json.contains("$comment")) {
        common_quality_json.at("$comment").get_to(common_quality.comment);
    }

    if (common_quality_json.contains("sdfRequired")) {
        common_quality_json.at("sdfRequired").get_to(common_quality.sdf_required);
    }
}

//! Function prototype for ParseDataQualities.
void ParseDataQualities(json& data_qualities_json, DataQuality& data_qualities);

//! Function used to parse data qualities from json to a DataQuality object.
DataQuality ParseDataQualities(json& data_qualities_json) {
    DataQuality data_quality;
    ParseDataQualities(data_qualities_json, data_quality);
    return data_quality;
}

//! Function used to parse a sdfChoice from json into a DataQuality map.
SdfChoice ParseSdfChoice(json& sdf_choice_json) {
    SdfChoice sdf_choice;
    // Iterate through all sdf_choice items and parse them individually
    for (const auto& data_quality_json : sdf_choice_json.items()){
        sdf_choice[data_quality_json.key()] = ParseDataQualities(data_quality_json.value());
    }

    return sdf_choice;
}

//! Function used to parse a Jso item type from json into a JsoItemType object.
JsoItem ParseJsoItem(json& jso_item_json) {
    JsoItem jso_item;
    // Parse the common qualities
    if (jso_item_json.contains("sdfRef")) {
        ResolveSdfRef(jso_item_json);
    }

    if (jso_item_json.contains("description")) {
        jso_item_json.at("description").get_to(jso_item.description);
    }

    if (jso_item_json.contains("$comment")) {
        jso_item_json.at("$comment").get_to(jso_item.comment);
    }

    // Parse general qualities
    if (jso_item_json.contains("type")) {
        jso_item_json.at("type").get_to(jso_item.type);
    }

    if (jso_item_json.contains("sdfChoice")) {
        jso_item.sdf_choice = ParseSdfChoice(jso_item_json.at("sdfChoice"));
    }

    if (jso_item_json.contains("enum")) {
        jso_item_json.at("enum").get_to(jso_item.enum_);
    }

    // Parse number and integer qualities
    if (jso_item_json.contains("minimum")) {
        jso_item_json.at("minimum").get_to(jso_item.minimum);
    }

    if (jso_item_json.contains("maximum")) {
        jso_item_json.at("maximum").get_to(jso_item.maximum);
    }

    // Parse string qualities
    if (jso_item_json.contains("format")) {
        jso_item_json.at("format").get_to(jso_item.format);
    }

    if (jso_item_json.contains("minLength")) {
        jso_item_json.at("minLength").get_to(jso_item.min_length);
    }

    if (jso_item_json.contains("maxLength")) {
        jso_item_json.at("maxLength").get_to(jso_item.max_length);
    }

    // Parse object qualities
    // Iterate through all items inside properties and parse them individually
    if (jso_item_json.contains("properties")){
        for (const auto& data_qualities_json : jso_item_json.at("properties").items()) {
            jso_item.properties[data_qualities_json.key()] = ParseDataQualities(data_qualities_json.value());
        }
    }
    if (jso_item_json.contains("required")) {
        jso_item_json.at("required").get_to(jso_item.required);
    }

    return jso_item;
}

//! Function used to parse data qualities from json into a DataQuality object.
void ParseDataQualities(json& data_qualities_json, DataQuality& data_quality) {
    // Parse the common qualities
    ParseCommonQualities(data_qualities_json, data_quality);

    // Parse general qualities
    if (data_qualities_json.contains("type")) {
        data_qualities_json.at("type").get_to(data_quality.type);
    }

    if (data_qualities_json.contains("sdfChoice")) {
        data_quality.sdf_choice = ParseSdfChoice(data_qualities_json.at("sdfChoice"));
    }

    if (data_qualities_json.contains("enum")) {
        data_qualities_json.at("enum").get_to(data_quality.enum_);
    }

    if (data_qualities_json.contains("const")) {
        data_qualities_json.at("const").get_to(data_quality.const_);
    }

    if (data_qualities_json.contains("default")) {
        data_qualities_json.at("default").get_to(data_quality.default_);
    }

    // Parse number and integer qualities
    if (data_qualities_json.contains("minimum")) {
        data_qualities_json.at("minimum").get_to(data_quality.minimum);
    }

    if (data_qualities_json.contains("maximum")) {
        data_qualities_json.at("maximum").get_to(data_quality.maximum);
    }

    if (data_qualities_json.contains("exclusiveMinimum")) {
        data_qualities_json.at("exclusiveMinimum").get_to(data_quality.exclusive_minimum);
    }

    if (data_qualities_json.contains("exclusiveMaximum")) {
        data_qualities_json.at("exclusiveMaximum").get_to(data_quality.exclusive_maximum);
    }

    if (data_qualities_json.contains("multipleOf")) {
        data_qualities_json.at("multipleOf").get_to(data_quality.multiple_of);
    }

    // Parse string qualities
    if (data_qualities_json.contains("minLength")) {
        data_qualities_json.at("minLength").get_to(data_quality.min_length);
    }

    if (data_qualities_json.contains("maxLength")) {
        data_qualities_json.at("maxLength").get_to(data_quality.max_length);
    }

    if (data_qualities_json.contains("pattern")) {
        data_qualities_json.at("pattern").get_to(data_quality.pattern);
    }

    if (data_qualities_json.contains("format")) {
        data_qualities_json.at("format").get_to(data_quality.format);
    }

    // Parse array qualities
    if (data_qualities_json.contains("minItems")) {
        data_qualities_json.at("minItems").get_to(data_quality.min_items);
    }

    if (data_qualities_json.contains("maxItems")) {
        data_qualities_json.at("maxItems").get_to(data_quality.max_items);
    }

    if (data_qualities_json.contains("uniqueItems")) {
        data_qualities_json.at("uniqueItems").get_to(data_quality.unique_items);
    }

    if (data_qualities_json.contains("items")) {
        data_quality.items = ParseJsoItem(data_qualities_json.at("items"));
    }

    // Parse object qualities
    // Iterate through all items inside properties and parse them individually
    if (data_qualities_json.contains("properties")){
        for (const auto& data_quality_json : data_qualities_json.at("properties").items()) {
            data_quality.properties[data_quality_json.key()] = ParseDataQualities(data_quality_json.value());
        }
    }
    if (data_qualities_json.contains("required")) {
        data_qualities_json.at("required").get_to(data_quality.required);
    }

    // Parse additional qualities
    if (data_qualities_json.contains("unit")) {
        data_qualities_json.at("unit").get_to(data_quality.unit);
    }

    if (data_qualities_json.contains("nullable")) {
        data_qualities_json.at("nullable").get_to(data_quality.nullable);
    }

    if (data_qualities_json.contains("sdfType")) {
        data_qualities_json.at("sdfType").get_to(data_quality.sdf_type);
    }

    if (data_qualities_json.contains("contentFormat")) {
        data_qualities_json.at("contentFormat").get_to(data_quality.content_format);
    }
}

//! Function used to parse a sdfEvent from json into a SdfEvent object.
SdfEvent ParseSdfEvent(json& sdf_event_json) {
    SdfEvent sdf_event;
    // Parse the common qualities
    ParseCommonQualities(sdf_event_json, sdf_event);

    // Parse the remaining fields
    if (sdf_event_json.contains("sdfOutputData")) {
        sdf_event.sdf_output_data = ParseDataQualities(sdf_event_json.at("sdfOutputData"));
    }

    // Iterate through all items inside sdf_data and parse them individually
    if (sdf_event_json.contains("sdfData")){
        for (const auto& sdf_data_json : sdf_event_json.at("sdfData").items()) {
            sdf_event.sdf_data[sdf_data_json.key()] = ParseDataQualities(sdf_data_json.value());
        }
    }

    return sdf_event;
}

//! Function used to parse a sdfAction from json into a SdfAction object.
SdfAction ParseSdfAction(json& sdf_action_json) {
    SdfAction sdf_action;
    // Parse the common qualities
    ParseCommonQualities(sdf_action_json, sdf_action);

    // Parse the remaining fields
    if (sdf_action_json.contains("sdfInputData")) {
        sdf_action.sdf_input_data = ParseDataQualities(sdf_action_json.at("sdfInputData"));
    }

    if (sdf_action_json.contains("sdfOutputData")) {
        sdf_action.sdf_output_data =  ParseDataQualities(sdf_action_json.at("sdfOutputData"));
    }

    // Iterate through all items inside sdf_data and parse them individually
    if (sdf_action_json.contains("sdfData")){
        for (const auto& sdf_data_json : sdf_action_json.at("sdfData").items()) {
            sdf_action.sdf_data[sdf_data_json.key()] = ParseDataQualities(sdf_data_json.value());
        }
    }

    return sdf_action;
}

//! Function used to parse a sdfProperty from json into a SdfProperty object.
SdfProperty ParseSdfProperty(json& sdf_property_json) {
    SdfProperty sdf_property;
    // Parse the data qualities
    ParseDataQualities(sdf_property_json, sdf_property);

    // Parse the remaining fields
    if (sdf_property_json.contains("readable")) {
        sdf_property_json.at("readable").get_to(sdf_property.readable);
    }

    if (sdf_property_json.contains("writable")) {
        sdf_property_json.at("writable").get_to(sdf_property.writable);
    }

    if (sdf_property_json.contains("observable")) {
        sdf_property_json.at("observable").get_to(sdf_property.observable);
    }

    return sdf_property;
}

//! Function used to parse a sdfObject from json into a SdfObject object.
SdfObject ParseSdfObject(json& sdf_object_json) {
    SdfObject sdf_object;
    // Parse the common qualities
    ParseCommonQualities(sdf_object_json, sdf_object);

    // Iterate through all sdfProperties and parse them individually
    if (sdf_object_json.contains("sdfProperty")){
        for (const auto& sdf_property_json : sdf_object_json.at("sdfProperty").items()) {
            sdf_object.sdf_property[sdf_property_json.key()] = ParseSdfProperty(sdf_property_json.value());
        }
    }

    // Iterate through all sdfActions and parse them individually
    if (sdf_object_json.contains("sdfAction")){
        for (const auto& sdf_action_json : sdf_object_json.at("sdfAction").items()) {
            sdf_object.sdf_action[sdf_action_json.key()] = ParseSdfAction(sdf_action_json.value());
        }
    }

    // Iterate through all sdfEvents and parse them individually
    if (sdf_object_json.contains("sdfEvent")){
        for (const auto& sdf_event_node : sdf_object_json.at("sdfEvent").items()) {
            sdf_object.sdf_event[sdf_event_node.key()] = ParseSdfEvent(sdf_event_node.value());
        }
    }

    // Iterate through all items inside sdf_data and parse them individually
    if (sdf_object_json.contains("sdfData")){
        for (const auto& data_quality_json : sdf_object_json.at("sdfData").items()) {
            sdf_object.sdf_data[data_quality_json.key()] = ParseDataQualities(data_quality_json.value());
        }
    }

    // Parse the remaining fields
    if (sdf_object_json.contains("minItems")) {
        sdf_object_json.at("minItems").get_to(sdf_object.min_items);
    }

    if (sdf_object_json.contains("maxItems")) {
        sdf_object_json.at("maxItems").get_to(sdf_object.max_items);
    }

    return sdf_object;
}

//! Function used to parse a sdfThing from json into a SdfThing object.
SdfThing ParseSdfThing(json& sdf_thing_json) {
    SdfThing sdf_thing;

    ParseCommonQualities(sdf_thing_json, sdf_thing);

    // Iterate through all sdfThings and parse them individually
    if (sdf_thing_json.contains("sdfThing")){
        for (const auto& nested_sdf_thing_json : sdf_thing_json.at("sdfThing").items()) {
            sdf_thing.sdf_thing[nested_sdf_thing_json.key()] = ParseSdfThing(nested_sdf_thing_json.value());
        }
    }

    // Iterate through all sdfObjects and parse them individually
    if (sdf_thing_json.contains("sdfObject")){
        for (const auto& sdf_object_json : sdf_thing_json.at("sdfObject").items()) {
            sdf_thing.sdf_object[sdf_object_json.key()] = ParseSdfObject(sdf_object_json.value());
        }
    }

    // Iterate through all sdfProperties and parse them individually
    if (sdf_thing_json.contains("sdfProperty")){
        for (const auto& sdf_property_json : sdf_thing_json.at("sdfProperty").items()) {
            sdf_thing.sdf_property[sdf_property_json.key()] = ParseSdfProperty(sdf_property_json.value());
        }
    }

    // Iterate through all sdfActions and parse them individually
    if (sdf_thing_json.contains("sdfAction")){
        for (const auto& sdf_action_json : sdf_thing_json.at("sdfAction").items()) {
            sdf_thing.sdf_action[sdf_action_json.key()] = ParseSdfAction(sdf_action_json.value());
        }
    }

    // Iterate through all sdfEvents and parse them individually
    if (sdf_thing_json.contains("sdfEvent")){
        for (const auto& sdf_event_json : sdf_thing_json.at("sdfEvent").items()) {
            sdf_thing.sdf_event[sdf_event_json.key()] = ParseSdfEvent(sdf_event_json.value());
        }
    }

    // Iterate through all items inside sdf_data and parse them individually
    if (sdf_thing_json.contains("sdfData")){
        for (const auto& data_quality_json : sdf_thing_json.at("sdfData").items()) {
            sdf_thing.sdf_data[data_quality_json.key()] = ParseDataQualities(data_quality_json.value());
        }
    }

    // Parse the remaining fields
    if (sdf_thing_json.contains("minItems")) {
        sdf_thing_json.at("minItems").get_to(sdf_thing.min_items);
    }

    if (sdf_thing_json.contains("maxItems")) {
        sdf_thing_json.at("maxItems").get_to(sdf_thing.max_items);
    }

    return sdf_thing;
}

//! Function used to parse a namespace block from json into a NamespaceBlock object.
NamespaceBlock ParseNamespaceBlock(json& namespace_block_json) {
    NamespaceBlock namespace_block;

    // Iterate through all namespace items and parse them individually
    for (const auto& namespace_item_json : namespace_block_json.at("namespace").items()) {
        namespace_block.namespaces[namespace_item_json.key()] = namespace_item_json.value();
    }
    namespace_block_json.at("defaultNamespace").get_to(namespace_block.default_namespace);

    return namespace_block;
}

//! Function used to parse an information block from json into a InformationBlock object.
InformationBlock ParseInformationBlock(json& info_block_json) {
    InformationBlock information_block;

    if (info_block_json.contains("title")) {
        info_block_json.at("title").get_to(information_block.title);
    }

    if (info_block_json.contains("description")) {
        info_block_json.find("description").value().get_to(information_block.description);
    }

    if (info_block_json.contains("version")) {
        info_block_json.at("version").get_to(information_block.version);
    }

    if (info_block_json.contains("modified")) {
        info_block_json.at("modified").get_to(information_block.modified);
    }

    if (info_block_json.contains("copyright")) {
        info_block_json.at("copyright").get_to(information_block.copyright);
    }

    if (info_block_json.contains("license")) {
        info_block_json.at("license").get_to(information_block.license);
    }

    if (info_block_json.contains("features")) {
        info_block_json.at("features").get_to(information_block.features);
    }

    if (info_block_json.contains("$comment")) {
        info_block_json.at("$comment").get_to(information_block.comment);
    }

    return information_block;
}

//! Function used to parse a sdf-model from json into a SdfModel object.
SdfModel ParseSdfModel(json& sdf_model_json) {
    SdfModel sdf_model;
    // Set the global sdf_model reference
    global_sdf_model = sdf_model_json;

    // Parse the information block
    if (sdf_model_json.contains("info")) {
        sdf_model.information_block = ParseInformationBlock(sdf_model_json.at("info"));
    }

    // Parse the namespace block
    if (sdf_model_json.contains("namespace")) {
        sdf_model.namespace_block = ParseNamespaceBlock(sdf_model_json);
    }

    // Parse the sdfThings
    if (sdf_model_json.contains("sdfThing")) {
        for (const auto& thing : sdf_model_json.at("sdfThing").items()) {
            sdf_model.sdf_thing[thing.key()] = ParseSdfThing(thing.value());
        }
    }

    // Parse the sdfObjects
    else if (sdf_model_json.contains("sdfObject")) {
        for (const auto &object: sdf_model_json.at("sdfObject").items()) {
            sdf_model.sdf_object[object.key()] = ParseSdfObject(object.value());
        }
    }
    // As described in Section 3.4 [https://datatracker.ietf.org/doc/draft-ietf-asdf-sdf/], SDF grants the possibility
    // to have sdf_property, sdf_action or sdf_event as a top level affordance. But as these are meant to be used as
    // re-usable definitions for usage via sdfRef, these are meaningless for the converter, hence they're ignored.

    return sdf_model;
}

//! Function used to parse a sdf-mapping from json into a SdfMapping object.
SdfMapping ParseSdfMapping(json& sdf_mapping_json) {
    SdfMapping sdf_mapping;
    // Parse the information block
    if (sdf_mapping_json.contains("info")) {
        sdf_mapping.information_block = ParseInformationBlock(sdf_mapping_json.at("info"));
    }

    // Parse the namespace block
    if (sdf_mapping_json.contains("namespace")) {
        sdf_mapping.namespace_block = ParseNamespaceBlock(sdf_mapping_json);
    }

    // Parse the map section
    if (sdf_mapping_json.contains("map")) {
        for (const auto& reference : sdf_mapping_json.at("map").items()) {
            for (const auto& field : reference.value().items()) {
                sdf_mapping.map[reference.key()][field.key()] = field.value();
            }
        }
    }

    return sdf_mapping;
}

//! Function used to serialize common qualities into the json format.
void SerializeCommonQualities(const CommonQuality& common_quality, json& common_quality_json) {
    if (!common_quality.description.empty()) {
        common_quality_json["description"] = common_quality.description;
    }

    if (!common_quality.label.empty()) {
        common_quality_json["label"] = common_quality.label;
    }

    if (!common_quality.comment.empty()) {
        common_quality_json["$comment"] = common_quality.comment;
    }

    if (!common_quality.sdf_ref.empty()) {
        common_quality_json["sdfRef"] = common_quality.sdf_ref;
    }

    if (!common_quality.sdf_required.empty()) {
        common_quality_json["sdfRequired"] = common_quality.sdf_required;
    }

}

//! Function prototype used for recursive calls.
json SerializeDataQualities(const DataQuality& data_quality);

//! Function used to serialize jso items into the json format.
json SerializeJsoItemType(const JsoItem& jso_item) {
    json jso_item_type_json;

    if (!jso_item.sdf_ref.empty()) {
        jso_item_type_json["sdfRef"] = jso_item.sdf_ref;
    }

    if (!jso_item.description.empty()) {
        jso_item_type_json["description"] = jso_item.description;
    }

    if (!jso_item.comment.empty()) {
        jso_item_type_json["$comment"] = jso_item.comment;
    }

    if (!jso_item.type.empty()) {
        jso_item_type_json["type"] = jso_item.type;
    }

    // Iterate through all fields and serialize them individually
    if (!jso_item.sdf_choice.empty()){
        json sdf_choice_json;
        for (const auto& sdf_choice_pair : jso_item.sdf_choice) {
            sdf_choice_json[sdf_choice_pair.first] = SerializeDataQualities(sdf_choice_pair.second);
            if (sdf_choice_json.at(sdf_choice_pair.first).empty())
                sdf_choice_json[sdf_choice_pair.first] = json::object();
        }
        jso_item_type_json["sdfChoice"] = sdf_choice_json;
    }

    if (!jso_item.enum_.empty()) {
        jso_item_type_json["enum"] = jso_item.enum_;
    }

    if (jso_item.minimum.has_value()) {
        jso_item_type_json["minimum"] = jso_item.minimum.value();
    }

    if (jso_item.maximum.has_value()) {
        jso_item_type_json["maximum"] = jso_item.maximum.value();
    }

    if (!jso_item.format.empty()) {
        jso_item_type_json["format"] = jso_item.format;
    }

    if (jso_item.min_length.has_value()) {
        jso_item_type_json["minLength"] = jso_item.min_length.value();
    }

    if (jso_item.max_length.has_value()) {
        jso_item_type_json["maxLength"] = jso_item.max_length.value();
    }

    if (!jso_item.properties.empty()) {
        json sdf_properties_json;
        for (const auto& data_quality_pair : jso_item.properties) {
            sdf_properties_json[data_quality_pair.first] = SerializeDataQualities(data_quality_pair.second);
        }
        jso_item_type_json["properties"] = sdf_properties_json;
    }

    if (!jso_item.required.empty()) {
        jso_item_type_json["required"] = jso_item.required;
    }

    return jso_item_type_json;
}

//! Function used to serialize data qualities into the json format.
void SerializeDataQualities(const DataQuality& data_quality, json& data_quality_json) {
    // Serialize common qualities
    SerializeCommonQualities(data_quality, data_quality_json);

    // Serialize the remaining fields
    if (!data_quality.type.empty()) {
        data_quality_json["type"] = data_quality.type;
    }

    // Iterate through all fields and serialize them individually
    if (!data_quality.sdf_choice.empty()){
        json sdf_choice_json;
        for (const auto& sdf_choice_pair : data_quality.sdf_choice) {
            sdf_choice_json[sdf_choice_pair.first] = SerializeDataQualities(sdf_choice_pair.second);
            if (sdf_choice_json.at(sdf_choice_pair.first).empty())
                sdf_choice_json[sdf_choice_pair.first] = json::object();
        }
        data_quality_json["sdfChoice"] = sdf_choice_json;
    }

    if (!data_quality.enum_.empty()) {
        data_quality_json["enum"] = data_quality.enum_;
    }

    if (data_quality.const_.has_value()) {
        data_quality_json["const"] = data_quality.const_;
    }

    if (data_quality.default_.has_value()) {
        data_quality_json["default"] = data_quality.default_.value();
    }

    if (data_quality.minimum.has_value()) {
        data_quality_json["minimum"] = data_quality.minimum.value();
    }

    if (data_quality.maximum.has_value()) {
        data_quality_json["maximum"] = data_quality.maximum.value();
    }

    if (data_quality.exclusive_minimum.has_value()) {
        data_quality_json["exclusiveMinimum"] = data_quality.exclusive_minimum.value();
    }

    if (data_quality.exclusive_maximum.has_value()) {
        data_quality_json["exclusiveMaximum"] = data_quality.exclusive_maximum.value();
    }

    if (data_quality.multiple_of.has_value()) {
        data_quality_json["multipleOf"] = data_quality.multiple_of.value();
    }

    if (data_quality.min_length.has_value()) {
        data_quality_json["minLength"] = data_quality.min_length.value();
    }

    if (data_quality.max_length.has_value()) {
        data_quality_json["maxLength"] = data_quality.max_length.value();
    }

    if (!data_quality.pattern.empty()) {
        data_quality_json["pattern"] = data_quality.pattern;
    }

    if (!data_quality.format.empty()) {
        data_quality_json["format"] = data_quality.format;
    }

    if (data_quality.min_items.has_value()) {
        data_quality_json["minItems"] = data_quality.min_items.value();
    }

    if (data_quality.max_items.has_value()) {
        data_quality_json["maxItems"] = data_quality.max_items.value();
    }

    if (data_quality.unique_items.has_value()) {
        data_quality_json["uniqueItems"] = data_quality.unique_items.value();
    }

    if (data_quality.items.has_value()) {
        data_quality_json["items"] = SerializeJsoItemType(data_quality.items.value());
    }

    if (!data_quality.properties.empty()) {
        json sdf_properties_json;
        for (const auto& data_quality_pair : data_quality.properties) {
            sdf_properties_json[data_quality_pair.first] = SerializeDataQualities(data_quality_pair.second);
        }
        data_quality_json["properties"] = sdf_properties_json;
    }

    if (!data_quality.required.empty()) {
        data_quality_json["required"] = data_quality.required;
    }

    if (!data_quality.unit.empty()) {
        data_quality_json["unit"] = data_quality.unit;
    }

    if (data_quality.nullable.has_value()) {
        data_quality_json["nullable"] = data_quality.nullable.value();
    }

    if (!data_quality.sdf_type.empty()) {
        data_quality_json["sdfType"] = data_quality.sdf_type;
    }

    if (!data_quality.content_format.empty()) {
        data_quality_json["contentFormat"] = data_quality.content_format;
    }
}

//! Function used to serialize data qualities into the json format.
json SerializeDataQualities(const DataQuality& data_quality) {
    json data_quality_json;
    SerializeDataQualities(data_quality, data_quality_json);
    return data_quality_json;
}

//! Function used to serialize sdfData into the json format.
json SerializeSdfData(const SdfData& sdf_data) {
    json sdf_data_json;
    // Iterate through all elements and serialize them individually
    for (const auto& data_quality_pair : sdf_data) {
        sdf_data_json[data_quality_pair.first] = SerializeDataQualities(data_quality_pair.second);
    }

    return sdf_data_json;
}

//! Function used to serialize a sdf_event into the json format.
json SerializeSdfEvent(const SdfEvent& sdf_event) {
    json sdf_event_json;
    // Serialize common qualities
    SerializeCommonQualities(sdf_event, sdf_event_json);

    // Serialize the sdfOutputData
    if (sdf_event.sdf_output_data.has_value()) {
        sdf_event_json["sdfOutputData"] = SerializeDataQualities(sdf_event.sdf_output_data.value());
    }

    // Serialize the sdfData elements
    if (!sdf_event.sdf_data.empty()) {
        sdf_event_json["sdfData"] = SerializeSdfData(sdf_event.sdf_data);
    }

    return sdf_event_json;
}

//! Function used to serialize a sdf_action into the json format.
json SerializeSdfAction(const SdfAction& sdf_action) {
    json sdf_action_json;
    // Serialize common qualities
    SerializeCommonQualities(sdf_action, sdf_action_json);

    // Serialize the sdfInputData
    if (sdf_action.sdf_input_data.has_value()) {
        sdf_action_json["sdfInputData"] = SerializeDataQualities(sdf_action.sdf_input_data.value());
    }

    // Serialize the sdfOutputData
    if (sdf_action.sdf_output_data.has_value()) {
        sdf_action_json["sdfOutputData"] = SerializeDataQualities(sdf_action.sdf_output_data.value());
    }

    // Serialize the sdfData elements
    if (!sdf_action.sdf_data.empty()) {
        sdf_action_json["sdfData"] = SerializeSdfData(sdf_action.sdf_data);
    }

    return sdf_action_json;
}

//! Function used to serialize a sdf_property into the json format.
json SerializeSdfProperty(const SdfProperty& sdf_property) {
    json sdf_property_json;
    // Serialize data qualities
    SerializeDataQualities(sdf_property, sdf_property_json);

    // Serialize the remaining fields
    if (sdf_property.readable.has_value()) {
        sdf_property_json["readable"] = sdf_property.readable.value();
    }

    if (sdf_property.writable.has_value()) {
        sdf_property_json["writable"] = sdf_property.writable.value();
    }

    if (sdf_property.observable.has_value()) {
        sdf_property_json["observable"] = sdf_property.observable.value();
    }

    return sdf_property_json;
}

//! Function used to serialize a sdf_object into the json format
json SerializeSdfObject(const SdfObject& sdf_object) {
     json sdf_object_json;
    // Serialize the common qualities
    SerializeCommonQualities(sdf_object, sdf_object_json);

    // Serialize the sdfProperties
    if (!sdf_object.sdf_property.empty()) {
        json sdf_property_json;
        for (const auto& sdf_property_pair: sdf_object.sdf_property) {
            sdf_property_json[sdf_property_pair.first] = SerializeSdfProperty(sdf_property_pair.second);
        }
        sdf_object_json["sdfProperty"] = sdf_property_json;
    }

    // Serialize the sdfActions
    if (!sdf_object.sdf_action.empty()) {
        json sdf_action_json;
        for (const auto& sdf_action_pair: sdf_object.sdf_action) {
            sdf_action_json[sdf_action_pair.first] = SerializeSdfAction(sdf_action_pair.second);
        }
        sdf_object_json["sdfAction"] = sdf_action_json;
    }

    // Serialize the sdfEvents
    if (!sdf_object.sdf_event.empty()) {
        json sdf_event_json;
        for (const auto& sdf_event_pair: sdf_object.sdf_event) {
            sdf_event_json[sdf_event_pair.first] = SerializeSdfEvent(sdf_event_pair.second);
        }
        sdf_object_json["sdfEvent"] = sdf_event_json;
    }

    // Serialize the sdf_data
    if (!sdf_object.sdf_data.empty()) {
        sdf_object_json["sdfData"] = SerializeSdfData(sdf_object.sdf_data);
    }

    // Serialize the remaining fields
    if (sdf_object.min_items.has_value()) {
        sdf_object_json["minItems"] = sdf_object.min_items.value();
    }

    if (sdf_object.max_items.has_value()) {
        sdf_object_json["maxItems"] = sdf_object.max_items.value();
    }

    return sdf_object_json;
}

//! Function used to serialize a sdfThing into the json format.
json SerializeSdfThing(const SdfThing& sdf_thing) {
    json sdf_thing_json;
    // Serialize the common qualities
    SerializeCommonQualities(sdf_thing, sdf_thing_json);

    // Serialize the sdfThings
    if (!sdf_thing.sdf_thing.empty()) {
        json nested_sdf_thing_json;
        for (const auto& sdf_thing_pair: sdf_thing.sdf_thing) {
            nested_sdf_thing_json[sdf_thing_pair.first] = SerializeSdfThing(sdf_thing_pair.second);
        }
        sdf_thing_json["sdfThing"] = nested_sdf_thing_json;
    }

    // Serialize the sdfObjects
    if (!sdf_thing.sdf_object.empty()) {
        json sdf_object_json;
        for (const auto& sdf_object_pair: sdf_thing.sdf_object) {
            sdf_object_json[sdf_object_pair.first] = SerializeSdfObject(sdf_object_pair.second);
        }
        sdf_thing_json["sdfObject"] = sdf_object_json;
    }

    // Serialize the sdfProperties
    if (!sdf_thing.sdf_property.empty()) {
        json sdf_property_json;
        for (const auto& sdf_property_pair: sdf_thing.sdf_property) {
            sdf_property_json[sdf_property_pair.first] = SerializeSdfProperty(sdf_property_pair.second);
        }
        sdf_thing_json["sdfProperty"] = sdf_property_json;
    }

    // Serialize the sdfActions
    if (!sdf_thing.sdf_action.empty()) {
        json sdf_action_json;
        for (const auto& sdf_action_pair: sdf_thing.sdf_action) {
            sdf_action_json[sdf_action_pair.first] = SerializeSdfAction(sdf_action_pair.second);
        }
        sdf_thing_json["sdfAction"] = sdf_action_json;
    }

    // Serialize the sdfEvents
    if (!sdf_thing.sdf_event.empty()) {
        json sdf_event_json;
        for (const auto& sdf_event_pair: sdf_thing.sdf_event) {
            sdf_event_json[sdf_event_pair.first] = SerializeSdfEvent(sdf_event_pair.second);
        }
        sdf_thing_json["sdfEvent"] = sdf_event_json;
    }

    // Serialize the sdfData
    if (!sdf_thing.sdf_data.empty())
        sdf_thing_json["sdfData"] = SerializeSdfData(sdf_thing.sdf_data);;

    // Serialize the remaining fields
    if (sdf_thing.min_items.has_value()) {
        sdf_thing_json["minItems"] = sdf_thing.min_items.value();
    }

    if (sdf_thing.max_items.has_value()) {
        sdf_thing_json["maxItems"] = sdf_thing.max_items.value();
    }

    return sdf_thing_json;
}

//! Function used to serialize a namespace block into the json format.
json SerializeNamespaceBlock(const NamespaceBlock& namespace_block) {
    json namespace_block_json;
    if (!namespace_block.namespaces.empty()) {
        namespace_block_json["namespace"] = namespace_block.namespaces;
    }

    if (!namespace_block.default_namespace.empty()) {
        namespace_block_json["default_namespace"] = namespace_block.default_namespace;
    }

    return namespace_block_json;
}

//! Function used to serialize an information block into the json format.
json SerializeInformationBlock(const InformationBlock& information_block) {
    json info_block_json;
    if (!information_block.title.empty()) {
        info_block_json["title"] = information_block.title;
    }

    if (!information_block.description.empty()) {
        info_block_json["description"] = information_block.description;
    }

    if (!information_block.version.empty()) {
        info_block_json["version"] = information_block.version;
    }

    if (!information_block.modified.empty()) {
        info_block_json["modified"] = information_block.modified;
    }

    if (!information_block.copyright.empty()) {
        info_block_json["copyright"] = information_block.copyright;
    }

    if (!information_block.license.empty()) {
        info_block_json["license"] = information_block.license;
    }

    if (!information_block.features.empty()) {
        info_block_json["features"] = information_block.features;
    }

    if (!information_block.comment.empty()) {
        info_block_json["comment"] = information_block.comment;
    }

    return info_block_json;
}

//! Function used to serialize a SdfModel object into the json format.
json SerializeSdfModel(const SdfModel& sdf_model) {
    json sdf_model_json;
    // Serialize the information block
    if (sdf_model.information_block.has_value()) {
        sdf_model_json["info"] = SerializeInformationBlock(sdf_model.information_block.value());
    }

    // Serialize the namespace block
    if (sdf_model.namespace_block.has_value()) {
        sdf_model_json.push_back(SerializeNamespaceBlock(sdf_model.namespace_block.value()));
    }

    // Serialize the sdfThing, if one is present
    if (!sdf_model.sdf_thing.empty()){
        json sdf_thing_json;
        for (const auto& sdf_thing_pair : sdf_model.sdf_thing) {
            sdf_thing_json[sdf_thing_pair.first] = SerializeSdfThing(sdf_thing_pair.second);
        }
        sdf_model_json["sdfThing"] = sdf_thing_json;
    }

    // Otherwise serialize the sdfObject
    else if (!sdf_model.sdf_object.empty()){
        json sdf_object_json;
        for (const auto& sdf_object_pair : sdf_model.sdf_object) {
            sdf_object_json[sdf_object_pair.first] = SerializeSdfObject(sdf_object_pair.second);
        }
        sdf_model_json["sdfObject"] = sdf_object_json;
    }

    return sdf_model_json;
}

//! Function used to serialize a SdfMapping object into the json format.
json SerializeSdfMapping(const SdfMapping& sdf_mapping) {
    json sdf_mapping_json;
    // Serialize the information block
    if (sdf_mapping.information_block.has_value()) {
        sdf_mapping_json["info"] = SerializeInformationBlock(sdf_mapping.information_block.value());
    }

    // Serialize the namespace block
    if (sdf_mapping.namespace_block.has_value()) {
        sdf_mapping_json.push_back(SerializeNamespaceBlock(sdf_mapping.namespace_block.value()));
    }

    // Serialize the mapping section
    sdf_mapping_json["map"] = sdf_mapping.map;
    return sdf_mapping_json;
}

} // namespace sdf
