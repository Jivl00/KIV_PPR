#include "svg_ploter.h"

double round_to_nearest(double value, int precision) {
    double scale = std::pow(10, precision);
    return std::round(value * scale) / scale;
}

// format the tick labels to remove unnecessary zeros
std::string format_tick_label(double value) {
    std::ostringstream oss;
    // if the number is a whole number (no decimals), display it as an integer
    if (value == static_cast<int>(value)) {
        oss << static_cast<int>(value);
    } else {
        // otherwise, display with up to 2 decimal places
        oss << std::setprecision(2) << value;
    }
    return oss.str();
}


void plot_graph(
        const std::vector<std::vector<double>> &x_points_all,
        const std::vector<std::vector<double>> &y_points_all,
        const std::vector<std::string> &line_labels,
        const std::string &x_label,
        const std::string &y_label,
        const std::string &title,
        const std::string &output_filename,
        const double canvas_width ,
        const double canvas_height
) {
    // ensure input points are valid
    if (x_points_all.size() != y_points_all.size() || x_points_all.empty()) {
        throw std::invalid_argument("Each set of x_points and y_points must be of the same size and not empty.");
    }

    // prepare the SVG output
    std::string svg_output;

    // create the SVG Renderer
    CSVG_Renderer svg_renderer(canvas_width, canvas_height, svg_output);

    // create the drawing object
    drawing::Drawing drawing;

    // get the root group
    drawing::Group &root = drawing.Root();

    // define margins for the plot area
    const double margin_left = 100;
    const double margin_bottom = 100;
    const double plot_width = canvas_width - margin_left - 50;
    const double plot_height = canvas_height - margin_bottom - 50;

    // calculate axis ranges based on all the points
    double x_min = std::numeric_limits<double>::infinity();
    double x_max = -std::numeric_limits<double>::infinity();
    double y_min = std::numeric_limits<double>::infinity();
    double y_max = -std::numeric_limits<double>::infinity();

    for (size_t i = 0; i < x_points_all.size(); ++i) {
        x_min = std::min(x_min, *std::min_element(x_points_all[i].begin(), x_points_all[i].end()));
        x_max = std::max(x_max, *std::max_element(x_points_all[i].begin(), x_points_all[i].end()));
        y_min = std::min(y_min, *std::min_element(y_points_all[i].begin(), y_points_all[i].end()));
        y_max = std::max(y_max, *std::max_element(y_points_all[i].begin(), y_points_all[i].end()));
    }

    // round the axis min/max values for neatness
    const double x_range = round_to_nearest(x_max - x_min, 1);
    const double y_range = round_to_nearest(y_max - y_min, 1);

    // adjust the axis limits to ensure room for labels
    const double x_axis_padding = x_range * 0.1;
    const double y_axis_padding = y_range * 0.1;
    const double x_adjusted_min = x_min - x_axis_padding;
    const double x_adjusted_max = x_max + x_axis_padding;
    const double y_adjusted_min = y_min - y_axis_padding;
    const double y_adjusted_max = y_max + y_axis_padding;

    // add axes
    auto &x_axis = root.Add<drawing::Line>(margin_left, canvas_height - margin_bottom, canvas_width - 50,
                                           canvas_height - margin_bottom);
    x_axis.Set_Stroke_Color(RGBColor::From_HTML_Color("#000000")).Set_Stroke_Width(2.0);

    auto &y_axis = root.Add<drawing::Line>(margin_left, canvas_height - margin_bottom, margin_left, 50);
    y_axis.Set_Stroke_Color(RGBColor::From_HTML_Color("#000000")).Set_Stroke_Width(2.0);

    // add grid and ticks
    const int num_ticks = 10;
    for (int i = 0; i <= num_ticks; ++i) {
        // grid lines and ticks for X axis
        double x_pos = margin_left + i * (plot_width / num_ticks);
        double x_value = x_adjusted_min + i * ((x_adjusted_max - x_adjusted_min) / num_ticks);

        // add grid line
        auto &x_grid = root.Add<drawing::Line>(x_pos, 50, x_pos, canvas_height - margin_bottom);
        x_grid.Set_Stroke_Color(RGBColor::From_HTML_Color("#CCCCCC")).Set_Stroke_Width(1.0);

        // add tick
        auto &x_tick = root.Add<drawing::Line>(x_pos, canvas_height - margin_bottom, x_pos,
                                               canvas_height - margin_bottom + 10);
        x_tick.Set_Stroke_Color(RGBColor::From_HTML_Color("#000000")).Set_Stroke_Width(2.0);

        // add label
        auto &x_label_text = root.Add<drawing::Text>(x_pos, canvas_height - margin_bottom + 25,
                                                     format_tick_label(x_value));
        x_label_text.Set_Font_Size(12).Set_Anchor(drawing::Text::TextAnchor::MIDDLE);
        x_label_text.Set_Transform("rotate(-20, " + std::to_string(x_pos) + ", " +
                                   std::to_string(canvas_height - margin_bottom + 25) + ")");
    }

    for (int i = 0; i <= num_ticks; ++i) {
        // grid lines and ticks for Y axis
        double y_pos = canvas_height - margin_bottom - i * (plot_height / num_ticks);
        double y_value = y_adjusted_min + i * ((y_adjusted_max - y_adjusted_min) / num_ticks);

        // add grid line
        auto &y_grid = root.Add<drawing::Line>(margin_left, y_pos, canvas_width - 50, y_pos);
        y_grid.Set_Stroke_Color(RGBColor::From_HTML_Color("#CCCCCC")).Set_Stroke_Width(1.0);

        // add tick
        auto &y_tick = root.Add<drawing::Line>(margin_left - 10, y_pos, margin_left, y_pos);
        y_tick.Set_Stroke_Color(RGBColor::From_HTML_Color("#000000")).Set_Stroke_Width(2.0);

        // add label
        auto &y_label_text = root.Add<drawing::Text>(margin_left - 20, y_pos, format_tick_label(y_value));
        y_label_text.Set_Font_Size(12).Set_Anchor(drawing::Text::TextAnchor::END);
    }

    // colors for each line
    std::vector<std::string> line_colors = {"#FF0000", "#0000FF", "#00FF00", "#FF00FF", "#00FFFF", "#FFFF00"};
    size_t color_index = 0;

    // plot each data series
    for (size_t i = 0; i < x_points_all.size(); ++i) {
        // initialize the first point for the polyline
        double x_first =
                margin_left + ((x_points_all[i][0] - x_adjusted_min) / (x_adjusted_max - x_adjusted_min)) * plot_width;
        double y_first = canvas_height - margin_bottom -
                         ((y_points_all[i][0] - y_adjusted_min) / (y_adjusted_max - y_adjusted_min)) * plot_height;

        // create polyline and add the first point
        auto &polyline = root.Add<drawing::PolyLine>(x_first, y_first);

        // iterate from the second point onwards
        for (size_t j = 1; j < x_points_all[i].size(); ++j) {
            double x = margin_left +
                       ((x_points_all[i][j] - x_adjusted_min) / (x_adjusted_max - x_adjusted_min)) * plot_width;
            double y = canvas_height - margin_bottom -
                       ((y_points_all[i][j] - y_adjusted_min) / (y_adjusted_max - y_adjusted_min)) * plot_height;
            polyline.Add_Point(x, y);
        }

        // set the polyline properties: transparent fill and semi-transparent stroke
        polyline.Set_Stroke_Color(RGBColor::From_HTML_Color(line_colors[color_index % line_colors.size()]))
                .Set_Stroke_Width(2.0).Set_Fill_Color(RGBColor::From_HTML_Color("#000000"))
                .Set_Fill_Opacity(0).Set_Stroke_Opacity(0.5); // set alpha to 0.5 for semi-transparent lines

        color_index++;
    }

    // add axis labels
    auto &x_axis_label = root.Add<drawing::Text>(canvas_width / 2, canvas_height - margin_bottom + 50, x_label);
    x_axis_label.Set_Font_Size(16).Set_Anchor(drawing::Text::TextAnchor::MIDDLE);

    auto &y_axis_label = root.Add<drawing::Text>(50, (canvas_height / 2) - 20, y_label);
    y_axis_label.Set_Font_Size(16).Set_Anchor(drawing::Text::TextAnchor::MIDDLE)
            .Set_Transform("rotate(-90, 50, " + std::to_string(canvas_height / 2) + ")");

    // add title
    auto &graph_title = root.Add<drawing::Text>(canvas_width / 2, 30, title);
    graph_title.Set_Font_Size(20).Set_Anchor(drawing::Text::TextAnchor::MIDDLE);

    // add legend for each line
    double legend_x = canvas_width - 50;
    double legend_y = 60;
    for (size_t i = 0; i < line_labels.size(); ++i) {
        auto &legend_line = root.Add<drawing::Line>(legend_x, legend_y + static_cast<double>(i) * 20, legend_x + 20, legend_y + static_cast<double>(i) * 20);
        legend_line.Set_Stroke_Color(RGBColor::From_HTML_Color(line_colors[i % line_colors.size()]))
                .Set_Stroke_Width(2.0);
        auto &legend_text = root.Add<drawing::Text>(legend_x + 30, legend_y + static_cast<double>(i) * 20, line_labels[i]);
        legend_text.Set_Font_Size(12).Set_Anchor(drawing::Text::TextAnchor::START);
    }

    // render the SVG
    svg_renderer.Begin_Render();
    drawing.Render(svg_renderer);
    svg_renderer.Finalize_Render();

    // save to file
    std::ofstream out_file(output_filename);
    if (out_file.is_open()) {
        out_file << svg_output;
        out_file.close();
    } else {
        throw std::runtime_error("Unable to write to output file: " + output_filename);
    }
}

void load_results(const std::string &filename,
                  std::unordered_map<std::string, std::unordered_map<std::string, std::vector<data_point>>> &data) {
    std::ifstream file(filename);
    std::string line, column, comp_type;
    double num_elements, CV, MAD, time;

    // skip the header
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::getline(ss, column, ',');
        ss >> num_elements;
        ss.ignore();
        std::getline(ss, comp_type, ',');
        ss >> CV;
        ss.ignore();
        ss >> MAD;
        ss.ignore();
        ss >> time;

        auto point = data_point{num_elements, time, CV, MAD};
        data[column][comp_type].emplace_back(point);
    }
}

void plot_results(const std::string& filename, const std::string& output_folder) {

    // load the results
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<data_point>>> data;

    load_results(filename, data);

    std::vector<std::string> column_labels = {"x", "y", "z"}; // columns to plot
    std::vector<std::string> comp_labels = {"CPU_sequential_vectorized", "CPU_sequential_no_vectorized", // computation types
                                            "CPU_parallel_vectorized", "CPU_parallel_no_vectorized", "GPU"};

    // traverse each column and plot the graphs
    for (const auto &column : column_labels) {
        std::vector<std::vector<double>> x_points_all;
        std::vector<std::vector<double>> y_points_all_time;
        std::vector<std::vector<double>> y_points_all_cv;
        std::vector<std::vector<double>> y_points_all_mad;


        for (const auto &comp : comp_labels) {
            std::vector<double> x_points;
            std::vector<double> y_points_time;
            std::vector<double> y_points_cv;
            std::vector<double> y_points_mad;

            for (const auto &point : data[column][comp]) {
                x_points.push_back(point.num_elements);
                y_points_time.push_back(point.time);
                y_points_cv.push_back(point.CV);
                y_points_mad.push_back(point.MAD);
            }

            x_points_all.push_back(x_points);
            y_points_all_time.push_back(y_points_time);
            y_points_all_cv.push_back(y_points_cv);
            y_points_all_mad.push_back(y_points_mad);
        }

        try {
            std::string output_filename = output_folder;
            plot_graph(x_points_all, y_points_all_time, comp_labels,
                       "Number of Elements", "Time (s)", "Computation time for column " + column,
                          output_filename.append("time_for_").append(column).append(".svg"));
            std::cout << "Graph saved to '" << output_filename << "'" << std::endl;
            output_filename = output_folder;
            plot_graph(x_points_all, y_points_all_cv, comp_labels,
                       "Number of Elements", "Coefficient of Variation", "Coefficient of Variation for column " + column,
                       output_filename.append("CV_for_").append(column).append(".svg"));
            std::cout << "Graph saved to '" << output_filename << "'" << std::endl;
            output_filename = output_folder;
            plot_graph(x_points_all, y_points_all_mad, comp_labels,
                       "Number of Elements", "Median Absolute Deviation", "Median Absolute Deviation for column " + column,
                       output_filename.append("MAD_for_").append(column).append(".svg"));
            std::cout << "Graph saved to '" << output_filename << "'" << std::endl;
        } catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
}
