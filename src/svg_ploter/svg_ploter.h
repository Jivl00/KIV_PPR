#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <string>
#include <stdexcept>
#include <iomanip>
#include <unordered_map>

#include "SVGRenderer.h"
#include "Drawing.h"

/**
 * Function to round a double to the nearest precision
 * @param value - value to round
 * @param precision - precision to round to
 * @return rounded value
 */
double round_to_nearest(double value, int precision = 2);


/**
 * Function to format a tick label
 * @param value - value to format
 * @return formatted value
 */
std::string format_tick_label(double value);


/**
 * Function to plot a graph using the SVGRenderer
 * @param x_points_all - vector of vectors of x points
 * @param y_points_all - vector of vectors of y points
 * @param line_labels - vector of line labels
 * @param x_label - x axis label
 * @param y_label - y axis label
 * @param title - title of the graph
 * @param output_filename - output filename
 * @param canvas_width - width of the canvas
 * @param canvas_height - height of the canvas
 */
void plot_graph(const std::vector<std::vector<double>> &x_points_all,
                const std::vector<std::vector<double>> &y_points_all,
                const std::vector<std::string> &line_labels = {},
                const std::string &x_label = "X Axis",
                const std::string &y_label = "Y Axis",
                const std::string &title = "Graph",
                const std::string &output_filename = "plot_graph.svg",
                double canvas_width = 800,
                double canvas_height = 600);

/**
 * Struct to store data points
 * num_elements - number of elements
 * time - time taken
 * CV - coefficient of variation
 * MAD - median absolute deviation
 */
struct data_point {
    double num_elements;
    double time;
    [[maybe_unused]] double CV;
    [[maybe_unused]] double MAD;
};

/**
 * Function to load results from a csv file
 * @param filename - filename to load
 * @param data - map to store the data
 */
void load_results(const std::string &filename,
                  std::unordered_map<std::string, std::unordered_map<std::string, std::vector<data_point>>> &data);


