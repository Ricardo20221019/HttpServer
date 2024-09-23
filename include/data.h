#ifndef DATA_H
#define DATA_H
#pragma once
#include <dirent.h>
#include <fstream>
#include <vector>
#include <set>
#include <list>
#include <cstdio>
#include <sstream>
#include <iostream>
#include <json.hpp>
using namespace std;
using namespace nlohmann;



struct TbMenu
{
    long long id;
    std::string path;
    std::string name;
    int sort;
    int meun_level;
    int parent_id;
};
struct TbMenuRole
{
    long long id;
    long long role_id;
    long long menu_id;
};
struct TbRole
{
    long long id;
    std::string role_name;
    std::string role_desc;
};
struct TbSetting
{
    long long id;
    std::string robot_no;
    std::string workspace;
    std::string version;
    std::string init_no;
    std::string init_map;
    std::string hardware_no;
    std::string setting;
};
struct TbUser
{
    long long id;
    std::string login_name;
    std::string password;
    std::string user_name;
    char status;
    long long role_id;
};

struct RoadMap
{
    std::string scene_name;
    std::string file_url;
};

struct GridMap
{
    std::string scene_name;
    std::string file_url;
    float resolution;
    float origin_x;
    float origin_y;
    float negate;
    float occupied_thresh;
    float free_thresh;
};
struct TbForbidden
{
    std::string scene_name;
    int index;
    json area;
};

struct CloudMap
{
    std::string scene_name;
    int index;
    std::string file_url;
    float min_x;
    float min_y;
    float min_z;
    float max_x;
    float max_y;
    float max_z;
};
struct TbRtkExtrin
{
    std::string scene_name;
    int rtk_state;
    double origin_lat;
    double origin_lon;
    float trans_x;
    float trans_y;
    float trans_z;
    float trans_roll;
    float trans_pitch;
    float trans_yaw;
};
struct TbTaskList
{
    std::string scene_name;
    std::string plan_name;
    int plan_type;
    int is_actived;
    std::string task_weeks;
    std::string task_times;
    std::string start_time;
    std::string end_time;
    int task_interval;
    int task_type;
    json task_list;
};
struct TbLatestPose
{
    std::string scene_name;
    float x;
    float y;
    float z;
    float roll;
    float pitch;
    float yaw;
};
struct TbPathsPoints
{
    std::string scene_name;
    std::string path_name;
    int id;
    float x;
    float y;
    float z;
    float yaw;
    int type;
    int turn_direction;
    float speed_limit;
};


#endif