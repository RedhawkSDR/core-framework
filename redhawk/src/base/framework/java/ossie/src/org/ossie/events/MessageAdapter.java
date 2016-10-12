/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK core.
 *
 * REDHAWK core is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

package org.ossie.events;

import org.ossie.properties.StructDef;

/**
 * Internal class to handle conversion of CORBA Any to message payloads and
 * dispatch to a listener
 */
class MessageAdapter<E extends StructDef> implements EventCallback<E> {
    public MessageAdapter(Class<E> structDef, MessageListener<E> listener)
    {
        this.structDef = structDef;
        this.listener = listener;
    }

    public void message(String messageId, org.omg.CORBA.Any any)
    {
        E messageData;
        try {
            messageData = this.structDef.newInstance();
        } catch (final Exception ex) {
            return;
        }

        messageData.fromAny(any);
        this.messageReceived(messageId, messageData);
    }

    public void messageReceived(String messageId, E messageData)
    {
        this.listener.messageReceived(messageId, messageData);
    }

    private Class<E> structDef;
    private MessageListener<E> listener;
}
