#include <iomanip>
#include "svg_ploter.h"

double round_to_nearest(double value, int precision = 2) {
    double scale = std::pow(10, precision);
    return std::round(value * scale) / scale;
}

// Format the tick labels to remove unnecessary zeros
std::string format_tick_label(double value) {
    std::ostringstream oss;
    // If the number is a whole number (no decimals), display it as an integer
    if (value == static_cast<int>(value)) {
        oss << static_cast<int>(value);
    } else {
        // Otherwise, display with up to 2 decimal places
        oss << std::fixed << std::setprecision(2) << value;
    }
    return oss.str();
}


void plot_graph(
        const std::vector<std::vector<double>>& x_points_all,
        const std::vector<std::vector<double>>& y_points_all,
        const std::vector<std::string>& line_labels = {},
        const std::string& x_label = "X Axis",
        const std::string& y_label = "Y Axis",
        const std::string& title = "Graph",
        const std::string& output_filename = "plot_graph.svg",
        const double canvas_width = 800,
        const double canvas_height = 600
) {
    // Ensure input points are valid
    if (x_points_all.size() != y_points_all.size() || x_points_all.empty()) {
        throw std::invalid_argument("Each set of x_points and y_points must be of the same size and not empty.");
    }

    // Prepare the SVG output
    std::string svg_output;

    // Create the SVG Renderer
    CSVG_Renderer svg_renderer(canvas_width, canvas_height, svg_output);

    // Create the drawing object
    drawing::Drawing drawing;

    // Get the root group
    drawing::Group& root = drawing.Root();

    // Define margins for the plot area
    const double margin_left = 100;
    const double margin_bottom = 100;
    const double plot_width = canvas_width - margin_left - 50;
    const double plot_height = canvas_height - margin_bottom - 50;

    // Calculate axis ranges based on all the points
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

    // Round the axis min/max values for neatness
    const double x_range = round_to_nearest(x_max - x_min, 1);
    const double y_range = round_to_nearest(y_max - y_min, 1);

    // Adjust the axis limits to ensure room for labels
    const double x_axis_padding = x_range * 0.1;
    const double y_axis_padding = y_range * 0.1;
    const double x_adjusted_min = x_min - x_axis_padding;
    const double x_adjusted_max = x_max + x_axis_padding;
    const double y_adjusted_min = y_min - y_axis_padding;
    const double y_adjusted_max = y_max + y_axis_padding;

    // Add axes
    auto& x_axis = root.Add<drawing::Line>(margin_left, canvas_height - margin_bottom, canvas_width - 50, canvas_height - margin_bottom);
    x_axis.Set_Stroke_Color(RGBColor::From_HTML_Color("#000000")).Set_Stroke_Width(2.0);

    auto& y_axis = root.Add<drawing::Line>(margin_left, canvas_height - margin_bottom, margin_left, 50);
    y_axis.Set_Stroke_Color(RGBColor::From_HTML_Color("#000000")).Set_Stroke_Width(2.0);

    // Add grid and ticks
    const int num_ticks = 10;
    for (int i = 0; i <= num_ticks; ++i) {
        // Grid lines and ticks for X axis
        double x_pos = margin_left + i * (plot_width / num_ticks);
        double x_value = x_adjusted_min + i * ((x_adjusted_max - x_adjusted_min) / num_ticks);

        // Add grid line
        auto& x_grid = root.Add<drawing::Line>(x_pos, 50, x_pos, canvas_height - margin_bottom);
        x_grid.Set_Stroke_Color(RGBColor::From_HTML_Color("#CCCCCC")).Set_Stroke_Width(1.0);

        // Add tick
        auto& x_tick = root.Add<drawing::Line>(x_pos, canvas_height - margin_bottom, x_pos, canvas_height - margin_bottom + 10);
        x_tick.Set_Stroke_Color(RGBColor::From_HTML_Color("#000000")).Set_Stroke_Width(2.0);

        // Add label
        auto& x_label_text = root.Add<drawing::Text>(x_pos, canvas_height - margin_bottom + 30, format_tick_label(x_value));
        x_label_text.Set_Font_Size(12).Set_Anchor(drawing::Text::TextAnchor::MIDDLE);
    }

    for (int i = 0; i <= num_ticks; ++i) {
        // Grid lines and ticks for Y axis
        double y_pos = canvas_height - margin_bottom - i * (plot_height / num_ticks);
        double y_value = y_adjusted_min + i * ((y_adjusted_max - y_adjusted_min) / num_ticks);

        // Add grid line
        auto& y_grid = root.Add<drawing::Line>(margin_left, y_pos, canvas_width - 50, y_pos);
        y_grid.Set_Stroke_Color(RGBColor::From_HTML_Color("#CCCCCC")).Set_Stroke_Width(1.0);

        // Add tick
        auto& y_tick = root.Add<drawing::Line>(margin_left - 10, y_pos, margin_left, y_pos);
        y_tick.Set_Stroke_Color(RGBColor::From_HTML_Color("#000000")).Set_Stroke_Width(2.0);

        // Add label
        auto& y_label_text = root.Add<drawing::Text>(margin_left - 20, y_pos, format_tick_label(y_value));
        y_label_text.Set_Font_Size(12).Set_Anchor(drawing::Text::TextAnchor::END);
    }

    // Colors for each line
    std::vector<std::string> line_colors = {"#FF0000", "#0000FF", "#00FF00", "#FF00FF"};
    size_t color_index = 0;

    // Plot each data series
    for (size_t i = 0; i < x_points_all.size(); ++i) {
        // Initialize the first point for the polyline
        double x_first = margin_left + ((x_points_all[i][0] - x_adjusted_min) / (x_adjusted_max - x_adjusted_min)) * plot_width;
        double y_first = canvas_height - margin_bottom - ((y_points_all[i][0] - y_adjusted_min) / (y_adjusted_max - y_adjusted_min)) * plot_height;

        // Create polyline and add the first point
        auto& polyline = root.Add<drawing::PolyLine>(x_first, y_first);

        // Iterate from the second point onwards
        for (size_t j = 1; j < x_points_all[i].size(); ++j) {
            double x = margin_left + ((x_points_all[i][j] - x_adjusted_min) / (x_adjusted_max - x_adjusted_min)) * plot_width;
            double y = canvas_height - margin_bottom - ((y_points_all[i][j] - y_adjusted_min) / (y_adjusted_max - y_adjusted_min)) * plot_height;
            polyline.Add_Point(x, y);
        }

        // Set the polyline properties: transparent fill and semi-transparent stroke
        polyline.Set_Stroke_Color(RGBColor::From_HTML_Color(line_colors[color_index % line_colors.size()]))
                .Set_Stroke_Width(2.0).Set_Fill_Color(RGBColor::From_HTML_Color("#000000"))
                .Set_Fill_Opacity(0).Set_Stroke_Opacity(0.5); // Set alpha to 0.5 for semi-transparent lines

        color_index++;
    }

    // Add axis labels
    auto& x_axis_label = root.Add<drawing::Text>(canvas_width / 2, canvas_height - margin_bottom + 50, x_label);
    x_axis_label.Set_Font_Size(16).Set_Anchor(drawing::Text::TextAnchor::MIDDLE);

    auto& y_axis_label = root.Add<drawing::Text>(50, (canvas_height / 2) - 20, y_label);
    y_axis_label.Set_Font_Size(16).Set_Anchor(drawing::Text::TextAnchor::MIDDLE)
            .Set_Transform("rotate(-90, 50, " + std::to_string(canvas_height / 2) + ")");

    // Add title
    auto& graph_title = root.Add<drawing::Text>(canvas_width / 2, 30, title);
    graph_title.Set_Font_Size(20).Set_Anchor(drawing::Text::TextAnchor::MIDDLE);

    // Add legend for each line
    double legend_x = canvas_width - 150;
    double legend_y = 60;
    for (size_t i = 0; i < line_labels.size(); ++i) {
        auto& legend_line = root.Add<drawing::Line>(legend_x, legend_y + i * 20, legend_x + 20, legend_y + i * 20);
        legend_line.Set_Stroke_Color(RGBColor::From_HTML_Color(line_colors[i % line_colors.size()]))
                .Set_Stroke_Width(2.0);
        auto& legend_text = root.Add<drawing::Text>(legend_x + 30, legend_y + i * 20, line_labels[i]);
        legend_text.Set_Font_Size(12).Set_Anchor(drawing::Text::TextAnchor::START);
    }

    // Render the SVG
    svg_renderer.Begin_Render();
    drawing.Render(svg_renderer);
    svg_renderer.Finalize_Render();

    // Save to file
    std::ofstream out_file(output_filename);
    if (out_file.is_open()) {
        out_file << svg_output;
        out_file.close();
    } else {
        throw std::runtime_error("Unable to write to output file: " + output_filename);
    }
}

//int main() {
//    // Data points for multiple lines
//    std::vector<std::vector<double>> x_points_all = {
//            {1, 2, 3, 4, 5},  // x-values for first line
//            {1, 2, 3, 4, 5},  // x-values for second line
//            {1, 2, 3, 4, 5}   // x-values for third line
//    };
//
//    std::vector<std::vector<double>> y_points_all = {
//            {1, 4, 9, 16, 25},  // y-values for first line
//            {5, 3, 9, 16, 2},   // y-values for second line
//            {1, 4, 7, 6, 5}     // y-values for third line
//    };
//
//    std::vector<std::string> line_labels = {"Line 1", "Line 2", "Line 3"};
//
//    try {
//        plot_graph(x_points_all, y_points_all, line_labels,
//                   "X Axis", "Y Axis", "Multiple Line Graph", "line_graph_multiple_lines.svg");
//        std::cout << "Graph saved to 'line_graph_multiple_lines.svg'" << std::endl;
//    } catch (const std::exception& e) {
//        std::cerr << "Error: " << e.what() << std::endl;
//    }
//
//    return 0;
//}
