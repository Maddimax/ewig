//
// ewig - an immutable text editor
// Copyright (C) 2017 Juan Pedro Bolivar Puente
//
// This file is part of ewig.
//
// ewig is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ewig is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ewig.  If not, see <http://www.gnu.org/licenses/>.
//

#pragma once

#include <ewig/keys.hpp>

#include <immer/box.hpp>
#include <immer/flex_vector.hpp>
#include <immer/vector.hpp>

#include <boost/optional.hpp>
#include <ctime>

namespace ewig {

using line = immer::flex_vector<wchar_t>;
using text = immer::flex_vector<line>;

using index = int;

struct coord
{
    index row;
    index col;

    bool operator<(const coord& other) const
    { return row < other.row || (row == other.row && col < other.col); }

    bool operator==(const coord& other) const
    { return row == other.row && col == other.col; }
    bool operator!=(const coord& other) const
    { return !(*this == other); }
};

struct file_buffer
{
    text content;
    coord cursor;
    coord scroll;
    boost::optional<coord> selection_start;
    immer::box<std::string> file_name;
    text file_content;
};

struct message
{
    std::time_t time_stamp;
    immer::box<std::string> content;
};

struct application
{
    file_buffer buffer;
    key_map keys;
    key_seq input;
    immer::vector<text> clipboard;
    immer::vector<message> messages;
};

using command = std::function<boost::optional<application>(application, coord)>;

constexpr auto tab_width = 8;

file_buffer load_file(const char* file_name);

coord actual_cursor(file_buffer buf);
coord actual_display_cursor(const file_buffer& buf);

index display_line_col(const line& ln, index col);

file_buffer page_up(file_buffer buf, coord size);
file_buffer page_down(file_buffer buf, coord size);

file_buffer move_line_start(file_buffer buf);
file_buffer move_line_end(file_buffer buf);
file_buffer move_buffer_start(file_buffer buf);
file_buffer move_buffer_end(file_buffer buf);

file_buffer move_cursor_up(file_buffer buf);
file_buffer move_cursor_down(file_buffer buf);
file_buffer move_cursor_left(file_buffer buf);
file_buffer move_cursor_right(file_buffer buf);

file_buffer scroll_to_cursor(file_buffer buf, coord wsize);

file_buffer delete_char(file_buffer buf);
file_buffer delete_char_right(file_buffer buf);

file_buffer insert_new_line(file_buffer buf);
file_buffer insert_tab(file_buffer buf);
file_buffer insert_char(file_buffer buf, wchar_t value);
file_buffer insert_text(file_buffer buf, text value);

std::pair<file_buffer, text> copy(file_buffer buf);
std::pair<file_buffer, text> cut(file_buffer buf);
std::pair<file_buffer, text> cut_rest(file_buffer buf);

coord editor_size(coord size);

file_buffer select_whole_buffer(file_buffer buf);
file_buffer start_selection(file_buffer buf);
file_buffer clear_selection(file_buffer buf);
std::tuple<coord, coord> selected_region(file_buffer buf);

application paste(application app, coord size);
application put_message(application state, std::string str);
application put_clipboard(application state, text content);

boost::optional<application> eval_command(application state, const std::string& cmd,
                                          coord editor_size);
application eval_insert_char(application state, wchar_t key, coord editor_size);

application clear_input(application state);
boost::optional<application> handle_key(application state, key_code key, coord size);

application apply_edit(application state, coord size, file_buffer edit);
application apply_edit(application state, coord size, text edit);
application apply_edit(application state, coord size, std::pair<file_buffer, text> edit);

template <typename Fn>
command edit_command(Fn fn)
{
    return [=] (application state, coord size) {
        return apply_edit(state, size, fn(state.buffer));
    };
}

template <typename Fn>
command paste_command(Fn fn)
{
    return [=] (application state, coord size) {
        return apply_edit(state, size, fn(state.buffer, state.clipboard.back()));
    };
}

template <typename Fn>
command scroll_command(Fn fn)
{
    return [=] (application state, coord wsize) {
        state.buffer = fn(state.buffer, wsize);
        return state;
    };
}

} // namespace ewig
