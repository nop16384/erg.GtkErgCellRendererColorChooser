/* gtkcellrenderertext.c
 * Copyright (C) 2000  Red Hat, Inc.,  Jonathan Blandford <jrb@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include "GtkErgCellRendererColorChooser.h"

#include <stdlib.h>

#include <gtk/gtkeditable.h>
#include <gtk/gtkentry.h>
#include <gtk/gtksizerequest.h>
#include <gtk/gtkmarshalers.h>
#include <gtk/gtkintl.h>
#include <gtk/gtkprivate.h>
#include <gtk/gtktreeprivate.h>

//#include <gtk/a11y/gtktextcellaccessible.h>
#include <gtk/gtk-a11y.h>


/**
 * SECTION:gtkcellrenderertext
 * @Short_description: Renders text in a cell
 * @Title: GtkErgCellRendererColorChooser
 *
 * A #GtkErgCellRendererColorChooser renders a given text in its cell, using the font, color and
 * style information provided by its properties. The text will be ellipsized if it is
 * too long and the #GtkErgCellRendererColorChooser:ellipsize property allows it.
 *
 * If the #GtkCellRenderer:mode is %GTK_CELL_RENDERER_MODE_EDITABLE,
 * the #GtkErgCellRendererColorChooser allows to edit its text using an entry.
 */


//  ************************************************************************************************
//  Declarations
//  ************************************************************************************************
static void gtk_erg_cell_renderer_color_chooser_finalize   (GObject                  *object);

static void gtk_erg_cell_renderer_color_chooser_get_property  (GObject                  *object,
						  guint                     param_id,
						  GValue                   *value,
						  GParamSpec               *pspec);

static void gtk_erg_cell_renderer_color_chooser_set_property(
    GObject                  *object,
    guint                     param_id,
    const GValue             *value,
    GParamSpec               *pspec);

static void gtk_erg_cell_renderer_color_chooser_render(
     GtkCellRenderer          *cell,
    cairo_t                  *cr,
    GtkWidget                *widget,
    const GdkRectangle       *background_area,
    const GdkRectangle       *cell_area,
    GtkCellRendererState      flags);

static GtkCellEditable *gtk_erg_cell_renderer_color_chooser_start_editing (GtkCellRenderer      *cell,
							      GdkEvent             *event,
							      GtkWidget            *widget,
							      const gchar          *path,
							      const GdkRectangle   *background_area,
							      const GdkRectangle   *cell_area,
							      GtkCellRendererState  flags);

static void       gtk_erg_cell_renderer_color_chooser_get_preferred_width            (GtkCellRenderer       *cell,
                                                                         GtkWidget             *widget,
                                                                         gint                  *minimal_size,
                                                                         gint                  *natural_size);
static void       gtk_erg_cell_renderer_color_chooser_get_preferred_height           (GtkCellRenderer       *cell,
                                                                         GtkWidget             *widget,
                                                                         gint                  *minimal_size,
                                                                         gint                  *natural_size);
static void       gtk_erg_cell_renderer_color_chooser_get_preferred_height_for_width (GtkCellRenderer       *cell,
                                                                         GtkWidget             *widget,
                                                                         gint                   width,
                                                                         gint                  *minimum_height,
                                                                         gint                  *natural_height);
static void       gtk_erg_cell_renderer_color_chooser_get_aligned_area               (GtkCellRenderer       *cell,
									 GtkWidget             *widget,
									 GtkCellRendererState   flags,
									 const GdkRectangle    *cell_area,
									 GdkRectangle          *aligned_area);


static          void    set_bg_color(
                            GtkErgCellRendererColorChooser  *   celltext    ,
                            GdkRGBA                         *   rgba        );

static          void     set_fg_color (
                            GtkErgCellRendererColorChooser  *   celltext    ,
                            GdkRGBA                         *   rgba        );

static inline   gboolean  show_placeholder_text(
                            GtkErgCellRendererColorChooser  *   celltext    );
//  ************************************************************************************************
//  Structs etc...
//  ************************************************************************************************
enum {
  EDITED,
  LAST_SIGNAL
};

enum {
  PROP_0,

  PROP_TEXT,
  //PROP_MARKUP,
  //PROP_ATTRIBUTES,
  //PROP_SINGLE_PARAGRAPH_MODE,
  //PROP_WIDTH_CHARS,
  //PROP_MAX_WIDTH_CHARS,
  //PROP_WRAP_WIDTH,
  //PROP_ALIGN,
  //PROP_PLACEHOLDER_TEXT,

  /* Style args */
  PROP_BACKGROUND,
  PROP_FOREGROUND,
  PROP_BACKGROUND_GDK,
  PROP_FOREGROUND_GDK,
  PROP_BACKGROUND_RGBA,
  PROP_FOREGROUND_RGBA,
  //PROP_FONT,
  //PROP_FONT_DESC,
  //PROP_FAMILY,
  //PROP_STYLE,
  //PROP_VARIANT,
  //PROP_WEIGHT,
  //PROP_STRETCH,
  //PROP_SIZE,
  //PROP_SIZE_POINTS,
  //PROP_SCALE,
  PROP_EDITABLE,
  //PROP_STRIKETHROUGH,
  //PROP_UNDERLINE,
  //PROP_RISE,
  //PROP_LANGUAGE,
  //PROP_ELLIPSIZE,
  //PROP_WRAP_MODE,

  /* Whether-a-style-arg-is-set args */
  PROP_BACKGROUND_SET,
  PROP_FOREGROUND_SET,
  //PROP_FAMILY_SET,
  //PROP_STYLE_SET,
  //PROP_VARIANT_SET,
  //PROP_WEIGHT_SET,
  //PROP_STRETCH_SET,
  //PROP_SIZE_SET,
  //PROP_SCALE_SET,
  PROP_EDITABLE_SET,
  //PROP_STRIKETHROUGH_SET,
  //PROP_UNDERLINE_SET,
  //PROP_RISE_SET,
  //PROP_LANGUAGE_SET,
  //PROP_ELLIPSIZE_SET,
  //PROP_ALIGN_SET,

  LAST_PROP
};

static guint            text_cell_renderer_signals  [LAST_SIGNAL];
static GParamSpec   *   text_cell_renderer_props    [LAST_PROP];

#define GTK_ERG_CELL_RENDERER_COLOR_CHOOSER_PATH "gtk-erg-cell-renderer-color-chooser"

struct _GtkErgCellRendererColorChooserPrivate
{
  GtkWidget *entry;

  PangoAttrList        *extra_attrs;
  GdkRGBA               foreground;
  GdkRGBA               background;
  PangoAlignment        align;
  PangoEllipsizeMode    ellipsize;
  PangoFontDescription *font;
  PangoLanguage        *language;
  PangoUnderline        underline_style;
  PangoWrapMode         wrap_mode;

  gchar *text;
  gchar *placeholder_text;

  gdouble font_scale;

  gint rise;
  gint fixed_height_rows;
  gint width_chars;
  gint max_width_chars;
  gint wrap_width;

  guint in_entry_menu     : 1;
  guint strikethrough     : 1;
  guint editable          : 1;
  guint scale_set         : 1;
  guint foreground_set    : 1;
  guint background_set    : 1;
  guint underline_set     : 1;
  guint rise_set          : 1;
  guint strikethrough_set : 1;
  guint editable_set      : 1;
  guint calc_fixed_height : 1;
  guint single_paragraph  : 1;
  guint language_set      : 1;
  guint markup_set        : 1;
  guint ellipsize_set     : 1;
  guint align_set         : 1;

  gulong focus_out_id;
  gulong populate_popup_id;
  gulong entry_menu_popdown_timeout;
};

G_DEFINE_TYPE_WITH_PRIVATE (GtkErgCellRendererColorChooser, gtk_erg_cell_renderer_color_chooser, GTK_TYPE_CELL_RENDERER)
//  ************************************************************************************************
//  Gtk boilerplate
//  ************************************************************************************************
#include    "crcc-bpl.ci"
//  ************************************************************************************************
//  GET / SET
//  ************************************************************************************************
#include    "crcc-get.ci"
#include    "crcc-set.ci"
//  ************************************************************************************************
//  FONTS
//  ************************************************************************************************
#include    "crcc-fonts.ci"
//  ************************************************************************************************
//  ...
//  ************************************************************************************************
static void
set_bg_color (GtkErgCellRendererColorChooser *celltext,
              GdkRGBA             *rgba)
{
  GtkErgCellRendererColorChooserPrivate *priv = celltext->priv;

  if (rgba)
    {
      if (!priv->background_set)
        {
          priv->background_set = TRUE;
          g_object_notify_by_pspec (G_OBJECT (celltext), text_cell_renderer_props[PROP_BACKGROUND_SET]);
        }

      priv->background = *rgba;
    }
  else
    {
      if (priv->background_set)
        {
          priv->background_set = FALSE;
          g_object_notify_by_pspec (G_OBJECT (celltext), text_cell_renderer_props[PROP_BACKGROUND_SET]);
        }
    }
}

static void
set_fg_color (GtkErgCellRendererColorChooser *celltext,
              GdkRGBA             *rgba)
{
  GtkErgCellRendererColorChooserPrivate *priv = celltext->priv;

  if (rgba)
    {
      if (!priv->foreground_set)
        {
          priv->foreground_set = TRUE;
          g_object_notify_by_pspec (G_OBJECT (celltext), text_cell_renderer_props[PROP_FOREGROUND_SET]);
        }

      priv->foreground = *rgba;
    }
  else
    {
      if (priv->foreground_set)
        {
          priv->foreground_set = FALSE;
          g_object_notify_by_pspec (G_OBJECT (celltext), text_cell_renderer_props[PROP_FOREGROUND_SET]);
        }
    }
}

static inline gboolean
show_placeholder_text (GtkErgCellRendererColorChooser *celltext)
{
  GtkErgCellRendererColorChooserPrivate *priv = celltext->priv;

  return priv->editable && priv->placeholder_text &&
    (!priv->text || !priv->text[0]);
}

static void
gtk_erg_cell_renderer_color_chooser_render (GtkCellRenderer      *cell,
			       cairo_t              *cr,
			       GtkWidget            *widget,
			       const GdkRectangle   *background_area,
			       const GdkRectangle   *cell_area,
			       GtkCellRendererState  flags)

{
  GtkErgCellRendererColorChooser *celltext = GTK_ERG_CELL_RENDERER_COLOR_CHOOSER (cell);
  GtkErgCellRendererColorChooserPrivate *priv = celltext->priv;
  GtkStyleContext *context;
  PangoLayout *layout;
  gint x_offset = 0;
  gint y_offset = 0;
  gint xpad, ypad;
  PangoRectangle rect;

    printf("(crcc)render\n");

  layout = get_layout (celltext, widget, cell_area, flags);
  get_size (cell, widget, cell_area, layout, &x_offset, &y_offset, NULL, NULL);
  context = gtk_widget_get_style_context (widget);

  //if (priv->background_set && (flags & GTK_CELL_RENDERER_SELECTED) == 0)
    {
        GdkRGBA   color;
        color.red   =   0.90;
        color.green =   0.20;
        color.blue  =   0.35;
        color.alpha =   1.0;

      gdk_cairo_rectangle (cr, background_area);
      gdk_cairo_set_source_rgba (cr, &priv->background);
      //gdk_cairo_set_source_rgba (cr, &color);
      cairo_fill (cr);
    }

  gtk_cell_renderer_get_padding (cell, &xpad, &ypad);

  if (priv->ellipsize_set && priv->ellipsize != PANGO_ELLIPSIZE_NONE)
    pango_layout_set_width (layout,
			    (cell_area->width - x_offset - 2 * xpad) * PANGO_SCALE);
  else if (priv->wrap_width == -1)
    pango_layout_set_width (layout, -1);

  pango_layout_get_pixel_extents (layout, NULL, &rect);
  x_offset = x_offset - rect.x;

  cairo_save (cr);

  gdk_cairo_rectangle (cr, cell_area);
  cairo_clip (cr);

  gtk_render_layout (context, cr,
                     cell_area->x + x_offset + xpad,
                     cell_area->y + y_offset + ypad,
                     layout);

  cairo_restore (cr);

  g_object_unref (layout);
}
//  ************************************************************************************************
//  EDITING
//  ************************************************************************************************
#include    "crcc-edit.ci"
//  ************************************************************************************************
//  Width Height etc...
//  ************************************************************************************************
/**
 * gtk_erg_cell_renderer_color_chooser_set_fixed_height_from_font:
 * @renderer: A #GtkErgCellRendererColorChooser
 * @number_of_rows: Number of rows of text each cell renderer is allocated, or -1
 *
 * Sets the height of a renderer to explicitly be determined by the “font” and
 * “y_pad” property set on it.  Further changes in these properties do not
 * affect the height, so they must be accompanied by a subsequent call to this
 * function.  Using this function is unflexible, and should really only be used
 * if calculating the size of a cell is too slow (ie, a massive number of cells
 * displayed).  If @number_of_rows is -1, then the fixed height is unset, and
 * the height is determined by the properties again.
 **/
void
gtk_erg_cell_renderer_color_chooser_set_fixed_height_from_font (GtkErgCellRendererColorChooser *renderer,
						   gint                 number_of_rows)
{
  GtkErgCellRendererColorChooserPrivate *priv;
  GtkCellRenderer *cell;

  g_return_if_fail (GTK_IS_CELL_RENDERER_TEXT (renderer));
  g_return_if_fail (number_of_rows == -1 || number_of_rows > 0);

  cell = GTK_CELL_RENDERER (renderer);
  priv = renderer->priv;

  if (number_of_rows == -1)
    {
      gint width, height;

      gtk_cell_renderer_get_fixed_size (cell, &width, &height);
      gtk_cell_renderer_set_fixed_size (cell, width, -1);
    }
  else
    {
      priv->fixed_height_rows = number_of_rows;
      priv->calc_fixed_height = TRUE;
    }
}

static void
gtk_erg_cell_renderer_color_chooser_get_preferred_width (GtkCellRenderer *cell,
                                            GtkWidget       *widget,
                                            gint            *minimum_size,
                                            gint            *natural_size)
{
  GtkErgCellRendererColorChooserPrivate *priv;
  GtkErgCellRendererColorChooser        *celltext;
  PangoLayout                *layout;
  PangoContext               *context;
  PangoFontMetrics           *metrics;
  PangoRectangle              rect;
  gint char_width, text_width, ellipsize_chars, xpad;
  gint min_width, nat_width;

    printf("(crcc)preferred w\n");

  /* "width-chars" Hard-coded minimum width:
   *    - minimum size should be MAX (width-chars, strlen ("..."));
   *    - natural size should be MAX (width-chars, strlen (label->text));
   *
   * "wrap-width" User specified natural wrap width
   *    - minimum size should be MAX (width-chars, 0)
   *    - natural size should be MIN (wrap-width, strlen (label->text))
   */

  celltext = GTK_ERG_CELL_RENDERER_COLOR_CHOOSER (cell);
  priv = celltext->priv;

  gtk_cell_renderer_get_padding (cell, &xpad, NULL);

  layout = get_layout (celltext, widget, NULL, 0);

  /* Fetch the length of the complete unwrapped text */
  pango_layout_set_width (layout, -1);
  pango_layout_get_extents (layout, NULL, &rect);
  text_width = rect.width;

  /* Fetch the average size of a charachter */
  context = pango_layout_get_context (layout);
  metrics = pango_context_get_metrics (context,
                                       pango_context_get_font_description (context),
                                       pango_context_get_language (context));

  char_width = pango_font_metrics_get_approximate_char_width (metrics);

  pango_font_metrics_unref (metrics);
  g_object_unref (layout);

  /* enforce minimum width for ellipsized labels at ~3 chars */
  if (priv->ellipsize_set && priv->ellipsize != PANGO_ELLIPSIZE_NONE)
    ellipsize_chars = 3;
  else
    ellipsize_chars = 0;

  if ((priv->ellipsize_set && priv->ellipsize != PANGO_ELLIPSIZE_NONE) || priv->width_chars > 0)
    min_width = xpad * 2 +
      MIN (PANGO_PIXELS_CEIL (text_width),
           (PANGO_PIXELS (char_width) * MAX (priv->width_chars, ellipsize_chars)));
  /* If no width-chars set, minimum for wrapping text will be the wrap-width */
  else if (priv->wrap_width > -1)
    min_width = xpad * 2 + rect.x + MIN (PANGO_PIXELS_CEIL (text_width), priv->wrap_width);
  else
    min_width = xpad * 2 + rect.x + PANGO_PIXELS_CEIL (text_width);

  if (priv->width_chars > 0)
    nat_width = xpad * 2 +
      MAX ((PANGO_PIXELS (char_width) * priv->width_chars), PANGO_PIXELS_CEIL (text_width));
  else
    nat_width = xpad * 2 + PANGO_PIXELS_CEIL (text_width);

  nat_width = MAX (nat_width, min_width);

  if (priv->max_width_chars > 0)
    {
      gint max_width = xpad * 2 + PANGO_PIXELS (char_width) * priv->max_width_chars;

      min_width = MIN (min_width, max_width);
      nat_width = MIN (nat_width, max_width);
    }

  if (minimum_size)
    *minimum_size = min_width;

  if (natural_size)
    *natural_size = nat_width;
}

static void
gtk_erg_cell_renderer_color_chooser_get_preferred_height_for_width (GtkCellRenderer *cell,
                                                       GtkWidget       *widget,
                                                       gint             width,
                                                       gint            *minimum_height,
                                                       gint            *natural_height)
{
  GtkErgCellRendererColorChooser *celltext;
  PangoLayout         *layout;
  gint                 text_height, xpad, ypad;

    printf("(crcc)preferred h for w\n");

  celltext = GTK_ERG_CELL_RENDERER_COLOR_CHOOSER (cell);

  gtk_cell_renderer_get_padding (cell, &xpad, &ypad);

  layout = get_layout (celltext, widget, NULL, 0);

  pango_layout_set_width (layout, (width - xpad * 2) * PANGO_SCALE);
  pango_layout_get_pixel_size (layout, NULL, &text_height);

  if (minimum_height)
    *minimum_height = text_height + ypad * 2;

  if (natural_height)
    *natural_height = text_height + ypad * 2;

  g_object_unref (layout);
}

static void
gtk_erg_cell_renderer_color_chooser_get_preferred_height (GtkCellRenderer *cell,
                                             GtkWidget       *widget,
                                             gint            *minimum_size,
                                             gint            *natural_size)
{
  gint min_width;

    printf("(crcc)preferred h\n");

  /* Thankfully cell renderers dont rotate, so they only have to do
   * height-for-width and not the opposite. Here we have only to return
   * the height for the base minimum width of the renderer.
   *
   * Note this code path wont be followed by GtkTreeView which is
   * height-for-width specifically.
   */
  gtk_cell_renderer_get_preferred_width (cell, widget, &min_width, NULL);
  gtk_erg_cell_renderer_color_chooser_get_preferred_height_for_width (cell, widget, min_width,
                                                         minimum_size, natural_size);
}

