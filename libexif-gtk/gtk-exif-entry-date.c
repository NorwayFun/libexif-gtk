/* gtk-exif-entry-date.c
 *
 * Copyright © 2001 Lutz Müller <lutz@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"
#include "gtk-exif-entry-date.h"
#include "gtk-exif-util.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>

#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (GETTEXT_PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

struct _GtkExifEntryDatePrivate {
	ExifEntry *entry;

	GtkCalendar *cal;
	GtkAdjustment *a_hour, *a_min, *a_sec;
};

#define PARENT_TYPE GTK_EXIF_TYPE_ENTRY
static GtkExifEntryClass *parent_class;

static void
#if GTK_CHECK_VERSION(3,0,0)
gtk_exif_entry_date_destroy (GtkWidget *widget)
#else
gtk_exif_entry_date_destroy (GtkObject *object)
#endif
{
#if GTK_CHECK_VERSION(3,0,0)
	GtkExifEntryDate *entry = GTK_EXIF_ENTRY_DATE (widget);
#else
	GtkExifEntryDate *entry = GTK_EXIF_ENTRY_DATE (object);
#endif

	if (entry->priv->entry) {
		exif_entry_unref (entry->priv->entry);
		entry->priv->entry = NULL;
	}

#if GTK_CHECK_VERSION(3,0,0)
	GTK_WIDGET_CLASS (parent_class)->destroy (widget);
#else
	GTK_OBJECT_CLASS (parent_class)->destroy (object);
#endif
}

GTK_EXIF_FINALIZE (entry_date, EntryDate)

static void
gtk_exif_entry_date_class_init (gpointer g_class, gpointer class_data G_GNUC_UNUSED)
{
#if GTK_CHECK_VERSION(3,0,0)
	GtkWidgetClass *widget_class;
	GObjectClass *gobject_class;

	widget_class = GTK_WIDGET_CLASS (g_class);
	widget_class->destroy = gtk_exif_entry_date_destroy;
#else
	GtkObjectClass *object_class;
	GObjectClass *gobject_class;

	object_class = GTK_OBJECT_CLASS (g_class);
	object_class->destroy  = gtk_exif_entry_date_destroy;
#endif

	gobject_class = G_OBJECT_CLASS (g_class);
	gobject_class->finalize = gtk_exif_entry_date_finalize;

	parent_class = g_type_class_peek_parent (g_class);
}

static void
gtk_exif_entry_date_init (GTypeInstance *instance, gpointer g_class G_GNUC_UNUSED)
{
	GtkExifEntryDate *entry = GTK_EXIF_ENTRY_DATE (instance);

	entry->priv = g_new0 (GtkExifEntryDatePrivate, 1);
}

GTK_EXIF_CLASS (entry_date, EntryDate, "EntryDate")

static void
gtk_exif_entry_date_load (GtkExifEntryDate *entry)
{
	GtkExifEntryDatePrivate *p;
	gchar *data;

	g_return_if_fail (GTK_EXIF_IS_ENTRY_DATE (entry));
	p = entry->priv;

	g_signal_handlers_block_matched (G_OBJECT (p->cal),
				G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, entry);
	g_signal_handlers_block_matched (G_OBJECT (p->a_hour),
				G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, entry);
	g_signal_handlers_block_matched (G_OBJECT (p->a_min),
				G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, entry);
	g_signal_handlers_block_matched (G_OBJECT (p->a_sec),
				G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, entry);

	data = g_strdup ((char *)entry->priv->entry->data);
	data[4] = data[7] = data[10] = data[13] = data[16] = '\0';
	gtk_calendar_select_month (p->cal, atoi (data + 5) - 1, atoi (data));
	gtk_calendar_select_day (entry->priv->cal, atoi (data + 8));
	gtk_adjustment_set_value (entry->priv->a_hour, atoi (data + 11));
	gtk_adjustment_set_value (entry->priv->a_min,  atoi (data + 14));
	gtk_adjustment_set_value (entry->priv->a_sec,  atoi (data + 17));
	g_free (data);

	g_signal_handlers_unblock_matched (G_OBJECT (p->cal),
				G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, entry);
	g_signal_handlers_unblock_matched (G_OBJECT (p->a_hour),
				G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, entry);
	g_signal_handlers_unblock_matched (G_OBJECT (p->a_min),
				G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, entry);
	g_signal_handlers_unblock_matched (G_OBJECT (p->a_sec),
				G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, entry);
}

static void
gtk_exif_entry_date_save (GtkExifEntryDate *entry)
{
	guint year, month, day;

	g_return_if_fail (GTK_EXIF_IS_ENTRY_DATE (entry));

	gtk_calendar_get_date (entry->priv->cal, &year, &month, &day);
	snprintf ((char *)entry->priv->entry->data, entry->priv->entry->size,
		 "%04i:%02i:%02i %02i:%02i:%02i",
		 (gint) year,
		 (gint) month + 1,
		 (gint) day,
		 (gint) gtk_adjustment_get_value(entry->priv->a_hour),
		 (gint) gtk_adjustment_get_value(entry->priv->a_min),
		 (gint) gtk_adjustment_get_value(entry->priv->a_sec));
	gtk_exif_entry_changed (GTK_EXIF_ENTRY (entry), entry->priv->entry);
}

static void
on_day_selected (GtkCalendar *calendar G_GNUC_UNUSED, GtkExifEntryDate *entry)
{
	gtk_exif_entry_date_save (entry);
}

static void
on_time_changed (GtkAdjustment *adjustment G_GNUC_UNUSED, GtkExifEntryDate *entry)
{
	gtk_exif_entry_date_save (entry);
}

GtkWidget *
gtk_exif_entry_date_new (ExifEntry *e)
{
	GtkExifEntryDate *entry;
	GtkWidget *c, *hbox, *label, *spin;
#if GTK_CHECK_VERSION(3,0,0)
	GtkAdjustment *a;
#else
	GtkObject *a;
#endif

	g_return_val_if_fail (e != NULL, NULL);
	g_return_val_if_fail ((e->tag == EXIF_TAG_DATE_TIME) ||
			      (e->tag == EXIF_TAG_DATE_TIME_ORIGINAL) ||
			      (e->tag == EXIF_TAG_DATE_TIME_DIGITIZED), NULL);

	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");

	entry = g_object_new (GTK_EXIF_TYPE_ENTRY_DATE, NULL);
	entry->priv->entry = e;
	exif_entry_ref (e);
	gtk_exif_entry_construct (GTK_EXIF_ENTRY (entry),
		exif_tag_get_title_in_ifd (e->tag, exif_content_get_ifd(e->parent)),
		exif_tag_get_description_in_ifd (e->tag, exif_content_get_ifd(e->parent)));

	c = gtk_calendar_new ();
	gtk_widget_show (c);
	gtk_box_pack_start (GTK_BOX (entry), c, TRUE, FALSE, 0);
	entry->priv->cal = GTK_CALENDAR (c);
	g_signal_connect (G_OBJECT (c), "day_selected",
			    G_CALLBACK (on_day_selected), entry);

	hbox = gtk_hbox_new (FALSE, 5);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (entry), hbox, TRUE, FALSE, 0);
	label = gtk_label_new (_("Time:"));
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	a = gtk_adjustment_new (0, 0, 23, 1, 1, 0);
	spin = gtk_spin_button_new (GTK_ADJUSTMENT (a), 0, 0);
	gtk_widget_show (spin);
	gtk_box_pack_start (GTK_BOX (hbox), spin, FALSE, FALSE, 0);
	entry->priv->a_hour = GTK_ADJUSTMENT (a);
	g_signal_connect (a, "value_changed",
			    G_CALLBACK (on_time_changed), entry);
	label = gtk_label_new (":");
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	a = gtk_adjustment_new (0, 0, 59, 1, 1, 0);
	spin = gtk_spin_button_new (GTK_ADJUSTMENT (a), 0, 0);
	gtk_widget_show (spin);
	gtk_box_pack_start (GTK_BOX (hbox), spin, FALSE, FALSE, 0);
	entry->priv->a_min = GTK_ADJUSTMENT (a);
	g_signal_connect (a, "value_changed",
			  G_CALLBACK (on_time_changed), entry);
	label = gtk_label_new (":");
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	a = gtk_adjustment_new (0, 0, 59, 1, 1, 0);
	spin = gtk_spin_button_new (GTK_ADJUSTMENT (a), 0, 0);
	gtk_widget_show (spin);
	gtk_box_pack_start (GTK_BOX (hbox), spin, FALSE, FALSE, 0);
	entry->priv->a_sec = GTK_ADJUSTMENT (a);
	g_signal_connect (a, "value_changed",
			  G_CALLBACK (on_time_changed), entry);

	gtk_exif_entry_date_load (entry);

	return (GTK_WIDGET (entry));
}
