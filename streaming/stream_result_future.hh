/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * Modified by Cloudius Systems.
 * Copyright 2015 Cloudius Systems.
 */

#pragma once

#include "core/sstring.hh"
#include "core/shared_ptr.hh"
#include "utils/UUID.hh"
#include "gms/inet_address.hh"
#include "streaming/stream_coordinator.hh"
#include "streaming/stream_event_handler.hh"
#include <vector>

namespace streaming {
    using UUID = utils::UUID;
    using inet_address = gms::inet_address;
/**
 * A future on the result ({@link StreamState}) of a streaming plan.
 *
 * In practice, this object also groups all the {@link StreamSession} for the streaming job
 * involved. One StreamSession will be created for every peer involved and said session will
 * handle every streaming (outgoing and incoming) to that peer for this job.
 * <p>
 * The future will return a result once every session is completed (successfully or not). If
 * any session ended up with an error, the future will throw a StreamException.
 * <p>
 * You can attach {@link StreamEventHandler} to this object to listen on {@link StreamEvent}s to
 * track progress of the streaming.
 */
class stream_result_future {
public:
    using UUID = utils::UUID;
    UUID plan_id;
    sstring description;
private:
    shared_ptr<stream_coordinator> _coordinator;
    std::vector<stream_event_handler*> _event_listeners;
public:
    stream_result_future(UUID plan_id_, sstring description_, bool keep_ss_table_levels_)
        : stream_result_future(plan_id_, description_, make_shared<stream_coordinator>(1, keep_ss_table_levels_)) {
        // Note: Origin sets connections_per_host = 0 on receiving side, We set 1 to
        // refelct the fact that we actaully create one conncetion to the initiator.
    }

    /**
     * Create new StreamResult of given {@code planId} and type.
     *
     * Constructor is package private. You need to use {@link StreamPlan#execute()} to get the instance.
     *
     * @param planId Stream plan ID
     * @param description Stream description
     */
    stream_result_future(UUID plan_id_, sstring description_, shared_ptr<stream_coordinator> coordinator_)
        : plan_id(std::move(plan_id_))
        , description(std::move(description_))
        , _coordinator(coordinator_) {
        // if there is no session to listen to, we immediately set result for returning
        if (!_coordinator->is_receiving() && !_coordinator->has_active_sessions()) {
            // set(getCurrentState());
        }
    }
public:
    shared_ptr<stream_coordinator> get_coordinator() { return _coordinator; };

public:
    static void init(UUID plan_id_, sstring description_, std::vector<stream_event_handler*> listeners_, shared_ptr<stream_coordinator> coordinator_);
    static void init_receiving_side(int session_index, UUID plan_id,
        sstring description, inet_address from, bool keep_ss_table_level);

private:
    static shared_ptr<stream_result_future> create_and_register(UUID plan_id_, sstring description_, shared_ptr<stream_coordinator> coordinator_) {
        auto future = make_shared<stream_result_future>(plan_id_, description_, coordinator_);
        // FIXME: StreamManager.instance.register(future);
        return future;
    }

public:
    void add_event_listener(stream_event_handler* listener) {
        // FIXME: Futures.addCallback(this, listener);
        _event_listeners.push_back(listener);
    }

#if 0
    /**
     * @return Current snapshot of streaming progress.
     */
    public StreamState getCurrentState()
    {
        return new StreamState(planId, description, coordinator.getAllSessionInfo());
    }

    @Override
    public boolean equals(Object o)
    {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        StreamResultFuture that = (StreamResultFuture) o;
        return planId.equals(that.planId);
    }

    @Override
    public int hashCode()
    {
        return planId.hashCode();
    }
#endif

    void handle_session_prepared(shared_ptr<stream_session> session);

#if 0

    void handleSessionComplete(StreamSession session)
    {
        logger.info("[Stream #{}] Session with {} is complete", session.planId(), session.peer);
        fireStreamEvent(new StreamEvent.SessionCompleteEvent(session));
        SessionInfo sessionInfo = session.getSessionInfo();
        coordinator.addSessionInfo(sessionInfo);
        maybeComplete();
    }

    public void handleProgress(ProgressInfo progress)
    {
        coordinator.updateProgress(progress);
        fireStreamEvent(new StreamEvent.ProgressEvent(planId, progress));
    }
#endif

    template <typename Event>
    void fire_stream_event(Event event);

#if 0
    private synchronized void maybeComplete()
    {
        if (!coordinator.hasActiveSessions())
        {
            StreamState finalState = getCurrentState();
            if (finalState.hasFailedSession())
            {
                logger.warn("[Stream #{}] Stream failed", planId);
                setException(new StreamException(finalState, "Stream failed"));
            }
            else
            {
                logger.info("[Stream #{}] All sessions completed", planId);
                set(finalState);
            }
        }
    }
#endif
};

} // namespace streaming
