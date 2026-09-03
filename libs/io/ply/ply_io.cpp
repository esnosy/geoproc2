#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <cassert>
#include <cstdint>
#include <cstdlib>

#include <fmt/os.h>

#include "../../file_buf.hpp"
#include "ply_io.hpp"

enum class PLY_Type { Char, UChar, Short, UShort, Int, UInt, Float, Double };
std::unordered_map<std::string, PLY_Type> str_to_ply_type = {
    {"char", PLY_Type::Char},     {"uchar", PLY_Type::UChar},
    {"short", PLY_Type::Short},   {"ushort", PLY_Type::UShort},
    {"int", PLY_Type::Int},       {"uint", PLY_Type::UInt},
    {"float", PLY_Type::Float},   {"double", PLY_Type::Double},
    {"int8", PLY_Type::Char},     {"uint8", PLY_Type::UChar},
    {"int16", PLY_Type::Short},   {"uint16", PLY_Type::UShort},
    {"int32", PLY_Type::Int},     {"uint32", PLY_Type::UInt},
    {"float32", PLY_Type::Float}, {"float64", PLY_Type::Double}};

struct Property {
  std::string name;
  // a double can hold all types stored in PLY, from 1 byte data types to 32
  // byte integers to float and of course double itself
  std::vector<std::vector<double>> values;
  enum class Type { List, Value };
  Type type;
  PLY_Type list_count_type;
  PLY_Type value_type;
};

struct Element {
  std::string name;
  std::vector<Property> props;
  size_t count;
};

bool starts_with(const char *str, const char *prefix) {
  while (*str && *prefix) {
    if (*str != *prefix) {
      return false;
    }
    str++;
    prefix++;
  }
  return *prefix == 0;
}

double read_value(std::ifstream &ifs, PLY_Type value_type) {
  switch (value_type) {
  case PLY_Type::Char:
    int8_t char_value;
    ifs.read(reinterpret_cast<char *>(&char_value), sizeof(int8_t));
    return char_value;
  case PLY_Type::UChar:
    uint8_t uchar_value;
    ifs.read(reinterpret_cast<char *>(&uchar_value), sizeof(uint8_t));
    return uchar_value;
  case PLY_Type::Short:
    int16_t short_value;
    ifs.read(reinterpret_cast<char *>(&short_value), sizeof(int16_t));
    return short_value;
  case PLY_Type::UShort:
    uint16_t ushort_value;
    ifs.read(reinterpret_cast<char *>(&ushort_value), sizeof(uint16_t));
    return ushort_value;
  case PLY_Type::Int:
    int32_t int_value;
    ifs.read(reinterpret_cast<char *>(&int_value), sizeof(int32_t));
    return int_value;
  case PLY_Type::UInt:
    uint32_t uint_value;
    ifs.read(reinterpret_cast<char *>(&uint_value), sizeof(uint32_t));
    return uint_value;
  case PLY_Type::Float:
    float float_value;
    ifs.read(reinterpret_cast<char *>(&float_value), sizeof(float));
    return float_value;
  case PLY_Type::Double:
    double double_value;
    ifs.read(reinterpret_cast<char *>(&double_value), sizeof(double));
    return double_value;
  }
  throw std::runtime_error("Unknown PLY type");
}

Indexed_Mesh read_ply(const char *filepath) {
  std::ifstream ifs(filepath, std::ios_base::binary);

  std::string token;
  std::string format;
  std::vector<Element> elements;

  while (ifs >> token) {
    if (token == "format") {
      ifs >> format;
    } else if (token == "element") {
      Element e;
      ifs >> e.name >> e.count;
      elements.push_back(e);
    } else if (token == "property") {
      Property p;
      ifs >> token;
      if (token == "list") {
        p.type = Property::Type::List;
        ifs >> token; // list count type
        p.list_count_type = str_to_ply_type.at(token);
        ifs >> token; // list value type
        p.value_type = str_to_ply_type.at(token);
      } else {
        p.type = Property::Type::Value;
        p.value_type = str_to_ply_type.at(token);
      }
      ifs >> p.name;
      if (elements.empty()) {
        constexpr const char *err = "PLY IO ERROR: Expected element before props";
        std::cerr << err << std::endl;
        throw std::runtime_error(err);
      }
      auto &e = elements.back();
      p.values.reserve(e.count);
      e.props.push_back(p);
    } else if (token == "end_header") {
      break;
    }
  }

  if (format == "binary_little_endian") {
    for (auto &e : elements) {
      for (size_t i = 0; i < e.count; i++) {
        for (auto &p : e.props) {
          if (p.type == Property::Type::List) {
            size_t list_size = (size_t)read_value(ifs, p.list_count_type);
            auto &values = p.values.emplace_back();
            values.reserve(list_size);
            for (size_t j = 0; j < list_size; j++) {
              values.push_back(read_value(ifs, p.value_type));
            }
          } else {
            double value = read_value(ifs, p.value_type);
            p.values.push_back({value});
          }
        }
      }
    }
  } else if (format == "ascii") {
    auto pos1 = ifs.tellg();
    ifs.seekg(0, std::ios_base::end);
    auto pos2 = ifs.tellg();
    auto buf_size = pos2 - pos1;
    ifs.seekg(pos1, std::ios_base::beg);
    char *buf = reinterpret_cast<char *>(malloc(buf_size));
    ifs.read(buf, buf_size);
    File_Buf file_buf(buf, buf_size);
    for (auto &e : elements) {
      for (size_t i = 0; i < e.count; i++) {
        for (auto &p : e.props) {
          if (p.type == Property::Type::List) {
            size_t list_size;
            file_buf.skip_spaces();
            file_buf.parse_token_as_number(list_size);
            auto &values = p.values.emplace_back();
            values.reserve(list_size);
            for (size_t j = 0; j < list_size; j++) {
              file_buf.skip_spaces();
              file_buf.parse_token_as_number(values.emplace_back());
            }
          } else {
            double value;
            file_buf.skip_spaces();
            file_buf.parse_token_as_number(value);
            p.values.push_back({value});
          }
        }
      }
    }

    free(buf);
  }

  std::vector<float> xs, ys, zs;

  Indexed_Mesh result;
  for (const auto &e : elements) {
    if (e.name == "vertex") {
      for (const auto &p : e.props) {
        if (p.name == "x") {
          for (const auto &x : p.values) {
            xs.push_back((float)x[0]);
          }
        } else if (p.name == "y") {
          for (const auto &y : p.values) {
            ys.push_back((float)y[0]);
          }
        } else if (p.name == "z") {
          for (const auto &z : p.values) {
            zs.push_back((float)z[0]);
          }
        }
      }
    } else if (e.name == "face") {
      for (const auto &p : e.props) {
        if (p.name == "vertex_indices" || p.name == "vertex_index") {
          result.faces.reserve(p.values.size());
          for (const auto &f : p.values) {
            std::vector<size_t> face;
            face.reserve(f.size());
            for (auto i : f) {
              face.push_back((size_t)i);
            }
            result.faces.push_back(face);
          }
        }
      }
    }
  }
  assert(xs.size() == ys.size() && ys.size() == zs.size());
  result.vertices.reserve(xs.size());
  for (size_t i = 0; i < xs.size(); i++) {
    result.vertices.push_back({xs[i], ys[i], zs[i]});
  }
  return result;
}

void write_ply_binary(const Indexed_Mesh &mesh, const char *filepath) {
  std::ofstream ofs(filepath, std::ios_base::binary | std::ios_base::trunc);
  ofs << "ply\n"
         "format binary_little_endian 1.0\n"
         "element vertex "
      << mesh.vertices.size()
      << "\n"
         "property float x\n"
         "property float y\n"
         "property float z\n"
         "element face "
      << mesh.faces.size()
      << "\n"
         "property list uchar uint vertex_indices\n"
         "end_header\n";
  ofs.write(reinterpret_cast<const char *>(mesh.vertices.data()),
            mesh.vertices.size() * sizeof Vec3);
  for (const auto &f : mesh.faces) {
    uint8_t num_verts = (uint8_t)f.size();
    ofs.write(reinterpret_cast<char *>(&num_verts), sizeof uint8_t);
    for (auto vi : f) {
      uint32_t vi_uint = (uint32_t)vi;
      ofs.write(reinterpret_cast<char *>(&vi_uint), sizeof uint32_t);
    }
  }
}

void write_ply_ascii(const Indexed_Mesh &mesh, const char *filepath) {
  auto out = fmt::output_file(filepath);
  out.print("ply\n"
            "format ascii 1.0\n"
            "element vertex {}\n"
            "property float x\n"
            "property float y\n"
            "property float z\n"
            "element face {}\n"
            "property list uchar uint vertex_indices\n"
            "end_header\n",
            mesh.vertices.size(), mesh.faces.size());
  for (auto &v : mesh.vertices) {
    out.print("{} {} {}\n", v.x, v.y, v.z);
  }
  for (auto &f : mesh.faces) {
    out.print("{}", f.size());
    for (auto vi : f) {
      out.print(" {}", vi);
    }
    out.print("\n");
  }
}