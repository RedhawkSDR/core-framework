/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK.
 *
 * REDHAWK is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * REDHAWK is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

$(document).ready(function () {
    // if sessionStorage has tree-of-files data:
    //     use it, because it has state info about the tree
    // else
    //     leave the original tree-of-files in place
    var data = window.sessionStorage.getItem("tree-of-files");
    if (data) {
        $('#tree-of-files').html(data);
    } else {
        $('.tree-of-files-children').removeClass("show").addClass("dontshow");
    }


    // Allow user to click section headings to show/hide their children.
    $('.tree-of-files-dir').click(function () {
        var triangle = $(this).children('.ui-icon');
        var children = $(this).next('.tree-of-files-children');
        if (children.hasClass('show')) {
            triangle.removeClass("ui-icon-triangle-1-s").addClass("ui-icon-triangle-1-e");
            children.removeClass("show").addClass("dontshow");
        } else if (children.hasClass('dontshow')) {
            triangle.removeClass("ui-icon-triangle-1-e").addClass("ui-icon-triangle-1-s");
            children.removeClass("dontshow").addClass("show");
        }
    });


    // Save tree-of-files to session storage.
    $('.save-tree').click(function () {
        var data = $('#tree-of-files').html();
        window.sessionStorage.setItem("tree-of-files", data);
    });
});
